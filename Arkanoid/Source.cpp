#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

const unsigned int windowWidth{ 800 }, windowHeight{ 600 };
const float ballRadius{ 10.f }, ballVelocity{ 8.f };
const float paddleWidth{ 60.f }, paddleHeight{ 20.f }, paddleVelocity{ 16.f };
const float blockWidth{ 60.f }, blockHeight{ 20.f };
const int countBlocksX{ 11 }, countBlocksY{ 4 };

struct Ball
{
	// CircleShape is a SFML class that
	// defines a renderable circle
	CircleShape shape;

	// 2D vector that stores the Ball's velocity
	Vector2f velocity{ -ballVelocity, -ballVelocity };


	// Ball constructor
	// mX starting X coord
	// mY starting Y coord
	Ball(float mX, float mY)
	{
		shape.setPosition(mX, mY);
		shape.setRadius(ballRadius);
		shape.setFillColor(Color::Red);
		shape.setOrigin(ballRadius, ballRadius);
	}

	void update()
	{
		shape.move(velocity);
		if (left() < 0) velocity.x = ballVelocity;
		else if (right() > windowWidth) velocity.x = -ballVelocity;

		if (top() < 0) velocity.y = ballVelocity;
		else if (bottom() > windowHeight) velocity.y = -ballVelocity;
	}

	float x()		{ return shape.getPosition().x; }
	float y()		{ return shape.getPosition().y; }
	float left()	{ return x() - shape.getRadius(); }
	float right()	{ return x() + shape.getRadius(); }
	float top()		{ return y() - shape.getRadius(); }
	float bottom()	{ return y() + shape.getRadius(); }

};


struct Paddle
{
	RectangleShape shape;
	Vector2f velocity;

	Paddle(float mX, float mY)
	{
		shape.setPosition(mX, mY);
		shape.setSize({ paddleWidth, paddleHeight });
		shape.setFillColor(Color::Red);
		shape.setOrigin(paddleWidth / 2.f, paddleHeight / 2.f);
	}

	void update()
	{
		shape.move(velocity);

		if (Keyboard::isKeyPressed(Keyboard::Key::Left) && left() > 0)
			velocity.x = -paddleVelocity;
		else if (Keyboard::isKeyPressed(Keyboard::Key::Right) && right() < windowWidth)
			velocity.x = paddleVelocity;
		else
			velocity.x = 0;
	}

	float x()		{ return shape.getPosition().x; }
	float y()		{ return shape.getPosition().y; }
	float left()	{ return x() - shape.getSize().x / 2.f; }
	float right()	{ return x() + shape.getSize().x / 2.f; }
	float top()		{ return y() - shape.getSize().y / 2.f; }
	float bottom()	{ return y() + shape.getSize().y / 2.f; }
};

struct Brick
{
	RectangleShape shape;
	bool destroyed{ false };

	Brick(float mX, float mY)
	{
		shape.setPosition(mX, mY);
		shape.setSize({ blockWidth, blockHeight });
		shape.setFillColor(Color::Yellow);
		shape.setOrigin(blockWidth / 2.f, blockHeight / 2.f);
	}

	float x()		{ return shape.getPosition().x; }
	float y()		{ return shape.getPosition().y; }
	float left()	{ return x() - shape.getSize().x / 2.f; }
	float right()	{ return x() + shape.getSize().x / 2.f; }
	float top()		{ return y() - shape.getSize().y / 2.f; }
	float bottom()	{ return y() + shape.getSize().y / 2.f; }
};

template<typename T1, typename T2> bool isIntersecting(T1& mA, T2& mB)
{
	return mA.right() >= mB.left() && mA.left() <= mB.right()
		&& mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}


void testCollision(Paddle& mPaddle, Ball& mBall)
{
	if (!isIntersecting(mPaddle, mBall))
		return;

	// let's push the ball upwards
	mBall.velocity.y = -ballVelocity;

	if (mBall.x() < mPaddle.x())
		mBall.velocity.x = -ballVelocity;
	else
		mBall.velocity.x = ballVelocity;
}

void testCollision(Brick& mBrick, Ball& mBall)
{
	if (!isIntersecting(mBrick, mBall)) return;

	mBrick.destroyed = true;

	// calculate how much the ball intersects the brick
	float overlapLeft{ mBall.right() - mBrick.left() };
	float overlapRight{ mBrick.right() - mBall.left() };
	float overlapTop{ mBall.bottom() - mBrick.top() };
	float overlapBottom{ mBrick.bottom() - mBall.top() };

	// if the magnitude of the left overlap is smaller than the
	// right one we can safely assume the ball hit the brick
	// from the left. The same for top/bottom collisions
	bool ballFromLeft(abs(overlapLeft) < abs(overlapRight));
	bool ballFromTop(abs(overlapTop) < abs(overlapBottom));

	// store the minimum overlaps for the X and Y axes
	float minOverlapX{ ballFromLeft ? overlapLeft : overlapRight };
	float minOverlapY{ ballFromTop ? overlapTop : overlapBottom };

	if (abs(minOverlapX) < abs(minOverlapY))
		mBall.velocity.x = ballFromLeft ? -ballVelocity : ballVelocity;
	else
		mBall.velocity.y = ballFromTop ? -ballVelocity : ballVelocity;

}
int main()
{
	RenderWindow window{ { windowWidth, windowHeight }, "Arkanoid - 1" };
	window.setFramerateLimit(60);

	Ball ball{ windowWidth / 2, windowHeight / 2 };
	Paddle paddle{ windowWidth / 2, windowHeight - 50 };
	vector<Brick> bricks;

	for (auto iX = 0; iX < countBlocksX; ++iX)
		for (auto iY = 0; iY < countBlocksY; ++iY)
			bricks.emplace_back((iX + 1) * (blockWidth + 3) + 22, (iY + 1)*(blockHeight + 3));

	//Game loop
	while (true)
	{
		Event event;
		window.pollEvent(event);

		window.clear(Color::Black);
		if (Keyboard::isKeyPressed(Keyboard::Key::Escape))
			break;

		// Every loop iteration, we need update the ball and paddle
		ball.update();
		paddle.update();
		testCollision(paddle, ball);

		for (auto& brick : bricks)
			testCollision(brick, ball);

		bricks.erase(remove_if(begin(bricks), end(bricks), [](const Brick& mBrick) {return mBrick.destroyed; }),
			end(bricks));

		window.draw(ball.shape);
		window.draw(paddle.shape);
		for (auto& brick : bricks)
			window.draw(brick.shape);

		window.display();
	}

	return 0;
}