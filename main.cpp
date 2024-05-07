#include <SFML/Graphics.hpp>
#include <random>

using namespace sf;

// Сетка
const double gridWidth = 1.5;
Color gridColor = Color(128, 128, 128);

// Скорость падения
double delayGlobal = 0.3;

// Разммер блока
const int blockSize = 35;

// Высота ширина
const int width = 10;
const int height = 20;

// Поле
int field[height][width]{ 0 };

// Структура для описания блоков
struct Point
{
	int x;
	int y;
} a[4], b[4];

// Массив фигур
int figures[7][4] =
{
	{1,3,5,7}, // I
	{2,4,5,7}, // Z
	{3,5,4,6}, // S
	{3,5,4,7}, // T
	{2,3,5,7}, // L
	{3,5,7,6}, // J
	{2,3,4,5}, // O
};

unsigned myRandom() {
	static unsigned seed = time(0); // зерно не должно быть 0
	seed = (seed * 73129 + 95121) % 100000;
	return seed;
}

void printGrid(RenderWindow& window) {
	RectangleShape gridLine(Vector2f(gridWidth, height * blockSize));
	gridLine.setFillColor(gridColor);

	gridLine.setPosition(0, 0);
	for (int i = 0; i < width + 1; ++i) {
		window.draw(gridLine);
		gridLine.move(blockSize, 0);
	}

	gridLine.rotate(-90);
	gridLine.setPosition(0, 0);
	for (int i = 0; i < height + 1; ++i) {
		window.draw(gridLine);
		gridLine.move(0, blockSize);
	}
}

Color getColor(int color) {
	switch (color)
	{
	case 1:
		return Color::Cyan;
	case 2:
		return Color::Red;
	case 3:
		return Color::Green;
	case 4:
		return Color(255, 0, 255); //purple
	case 5:
		return Color(255, 127, 80); //orange
	case 6:
		return Color::Blue;
	case 7:
		return Color::Yellow;
	}

	return Color::White;
}

// Проверка колизии
bool check(Point* a) {
	for (int i = 0; i < 4; ++i) {
		if (a[i].x >= width || a[i].x < 0 || a[i].y > height) return 0;
		else if (field[a[i].y][a[i].x]) return 0;
	}

	return 1;
}

// Создание новой фигуры
void newFigure(int& figure, int& color, bool& gameIsOver) {
	figure = myRandom() % 7;
	color = figure + 1;
	for (int i = 0; i < 4; ++i) {
		a[i].x = figures[figure][i] % 2 + width / 2 - 1;
		a[i].y = figures[figure][i] / 2;
	}

	for (int i = 0; i < 4; ++i) {
		if (field[a[i].y][a[i].x]) {
			gameIsOver = 1;
			break;
		}
	}
}

void checkLine() {
	int k = height - 1;
	for (int i = height - 1; i > 0; --i) {
		int count = 0;
		for (int j = 0; j < width; ++j) {
			if (field[i][j]) ++count;
			field[k][j] = field[i][j];
		}
		if (count < width)
			--k;
	}
}


int main() {

	// Создание окна
	RenderWindow window(VideoMode(blockSize * width + gridWidth, blockSize * height + gridWidth), "ed Tetris");
	window.setVerticalSyncEnabled(1);

	bool gameIsOver = 0;

	int figure;
	int color;

	int dx = 0;
	int rotate = 0;
	bool fastDown = 0;

	int addDelay = 0;

	// Инит первой фигуры
	newFigure(figure, color, gameIsOver);

	// Таймер
	Clock clock;
	double timer = 0;

	while (window.isOpen()) {

		// Работа с таймером
		double temp = clock.getElapsedTime().asSeconds();
		clock.restart();
		timer += temp;

		// Обнуление delay
		double delay = delayGlobal;

		// Обработка ввода
		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) {
				window.close();
			}
			else if (event.type == Event::KeyPressed) {
				switch (event.key.code) {
				case Keyboard::Right:
					dx = 1;
					break;

				case Keyboard::Left:
					dx = -1;
					break;

				case Keyboard::C:
					rotate = 1;
					break;

				case Keyboard::X:
					rotate = -1;
					break;

				case Keyboard::Z:
					rotate = 2;
					break;

				case Keyboard::Space:
					fastDown = 1;
					break;
				}
			}
		}
		if (Keyboard::isKeyPressed(Keyboard::Down))
			delay = 0.05;

		// Вращение
		if (rotate)
		{
			Point p = a[1]; // задаем центр вращения

			if (rotate != 2) {
				for (int i = 0; i < 4; i++)
				{
					int x = a[i].y - p.y; // y-y0
					int y = a[i].x - p.x; // x-x0
					a[i].x = p.x - x * (rotate);
					a[i].y = p.y + y * (rotate);
				}
			}
			else
			{
				for (int i = 0; i < 8; i++)
				{
					int x = a[i % 4].y - p.y; // y-y0
					int y = a[i % 4].x - p.x; // x-x0
					a[i % 4].x = p.x - x;
					a[i % 4].y = p.y + y;
				}
			}

			// Вышли за пределы поля после поворота? Тогда возвращаем старые координаты 
			if (!check(a)) { for (int i = 0; i < 4; i++) a[i] = b[i]; }

			rotate = 0;
		}

		// Смещение по x
		if (dx) {
			for (int i = 0; i < 4; ++i) {
				b[i] = a[i];
				a[i].x += dx;
			}
			if (!check(a)) {
				for (int i = 0; i < 4; ++i) {
					a[i] = b[i];
				}
			}
			dx = 0;
		}

		// space
		if (fastDown) {
			while (check(a)) {
				for (int i = 0; i < 4; ++i) {
					b[i] = a[i];
					a[i].y += 1;
				}
			}

			for (int i = 0; i < 4; ++i) {
				a[i] = b[i];
			}

			for (int i = 0; i < 4; ++i) {
				field[b[i].y][b[i].x] = color;
			}

			newFigure(figure, color, gameIsOver);

			checkLine();

			fastDown = 0;
			timer = 0;
			addDelay = 0;
		}

		// Обработка падения
		if (timer > delay) {
			for (int i = 0; i < 4; ++i) {
				b[i] = a[i];
				a[i].y += 1;
			}
			if (!check(a)) {
				for (int i = 0; i < 4; ++i) {
					a[i] = b[i];
				}
				++addDelay;
			}

			if (addDelay == 2) {
				for (int i = 0; i < 4; ++i) {
					field[b[i].y][b[i].x] = color;
				}
				addDelay = 0;

				newFigure(figure, color, gameIsOver);

				checkLine();
			}

			timer = 0;
		}

		// Отрисовка
		window.clear(Color::Black);
		RectangleShape rectangle(Vector2f(blockSize, blockSize));
		for (int i = 0; i < height; ++i) {
			for (int j = 0; j < width; ++j) {
				if (field[i][j]) {
					rectangle.setFillColor(getColor(field[i][j]));
					rectangle.setPosition(j * blockSize + gridWidth, i * blockSize);
					window.draw(rectangle);
				}
			}
		}
		for (int i = 0; i < 4; ++i) {
			rectangle.setFillColor(getColor(color));
			rectangle.setPosition(a[i].x * blockSize + gridWidth, a[i].y * blockSize);
			window.draw(rectangle);
		}
		printGrid(window);

		// Проверка на конец игры
		if (gameIsOver) {
			sf::Text text;

			Font font;
			font.loadFromFile("PoetsenOne-Regular.ttf");
			text.setFont(font);
			text.setString("Game is over!");
			text.setCharacterSize(blockSize * 1.3);
			text.setFillColor(Color::Red);
			text.setStyle(sf::Text::Bold);
			text.setPosition(Vector2f(width / 2 * blockSize / 4 - blockSize / 4, height / 2 * blockSize - 2 * blockSize));

			window.draw(text);

			text.setCharacterSize(blockSize * 1.3 - 2);
			text.setFillColor(sf::Color(255, 255, 255));
			text.move(blockSize * 0.2, 0);

			window.draw(text);

			window.display();

			while (window.isOpen()) {
				while (window.pollEvent(event)) {
					if (event.type == Event::Closed) {
						window.close();
					}
				}
			}
		}

		window.display();
	}

	return 0;
}
