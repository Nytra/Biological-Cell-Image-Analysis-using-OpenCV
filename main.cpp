#include <iostream>
#include <SFML\Graphics.hpp>
#include "constants.h"

struct Coord {
	int x;
	int y;
};

class Cluster {
private:
	std::vector<Coord> _coords;
public:
	Cluster(std::vector<Coord> coords) {
		_coords = coords;
	}

	Cluster() {}
	
	void addCoord(Coord c) {
		_coords.push_back(c);
	}

	std::vector<Coord> getCoords() {
		return _coords;
	}

	void clear() {
		_coords.clear();
	}
};

// scans the whole image for clusters and stores them in the coords vector
std::vector<Coord> findClusters(int *pixels, double tolerance, int width, int height) {
	std::vector<Coord> coords;
	int v, cx, cy;
	int sum = 0;
	int pixelCount = width * height;
	int count = 0;
	int scanSize = width / 100;

	for (int i = 0; i < pixelCount; i++) {

		if (pixels[i] > 0) {
			count += 1;
			sum += pixels[i];
		}
	}

	int threshold = (sum / count) * tolerance;  // average pixel value multiplied by the inverse of the tolerance

	for (int y = 0; y < height - scanSize; y += scanSize) {

		for (int x = 0; x < width - scanSize; x += scanSize) {
			sum = 0;
			count = 0;

			for (int j = y; j < y + scanSize; j++) {

				for (int k = x; k < x + scanSize; k++) {
					v = pixels[(j * width) + k];
					sum += v;
					count += 1;
				}
			}

			if (count > 0 && sum / count > threshold) {
				Coord coord;
				coord.x = x;
				coord.y = y;
				coords.push_back(coord);
			}
		}
	}

	return coords;
}

int main() {
	sf::Image image;
	sf::Font font;

	if (!image.loadFromFile("test.png")) {
		return -1;
	}
	std::cout << "Image loaded." << std::endl;

	if (!font.loadFromFile("font.ttf")) {
		return -1;
	}
	
	const int imageWidth = image.getSize().x;
	const int imageHeight = image.getSize().y;

	std::cout << "Width: " << imageWidth << std::endl;
	std::cout << "Height: " << imageHeight << std::endl;

	sf::RenderWindow window(sf::VideoMode(imageWidth, imageHeight), "Application");
	window.setFramerateLimit(30);

	sf::Text text;
	text.setFont(font);
	text.setFillColor(sf::Color::Black);
	text.setCharacterSize(40);

	int *pixels = new int[imageWidth * imageHeight];
	bool processed = false;
	int scanCount = 100;
	int scanSize = 10;

	sf::RectangleShape cluster(sf::Vector2f(scanSize - 1, scanSize - 1));
	cluster.setFillColor(sf::Color::Red);

	sf::RectangleShape pixel(sf::Vector2f(1, 1));

	std::vector<Coord> coords;

	

	while (window.isOpen()) {

		sf::Event event;
		while (window.pollEvent(event)) {

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		window.clear(sf::Color::White);
		int i = 0;

		for (int y = 0; y < imageHeight; y++) {

			for (int x = 0; x < imageWidth; x++) {
				pixels[i] = image.getPixel(x, y).b;
				i += 1;
			}
		}

		for (int y = 0; y < imageHeight; y++) {

			for (int x = 0; x < imageWidth; x++) {
				int value = image.getPixel(x, y).b;

				if (value > 0) {
					pixel.setFillColor(sf::Color(0, 0, value));
					pixel.setPosition(x * (imageWidth / windowWidth), y * (imageHeight / windowHeight));
					window.draw(pixel);
				}
			}
		}

		
		if (!processed) {
			
			processed = true;
			coords = findClusters(pixels, tolerance, imageWidth, imageHeight);
			std::cout << "Found " << coords.size() << " coords." << std::endl;

			// start cluster logic

			std::vector<Coord> checked;
			Coord c;
			int count = 0;

			// use a while loop to make a recursive path finder which keeps going until there is nowhere else to go
			for (int i = 0; i < coords.size(); i++) {

				break;

				for (int j = 0; j < coords.size(); j++) {

					if (i == j) {
						continue;
					}

					bool found = false;
					for (int k = 0; k < checked.size(); k++) {
						if (checked[k].x == coords[i].x || checked[k].y == coords[i].y) {
							found = true;
						}
						if (checked[k].x == coords[j].x || checked[k].y == coords[j].y) {
							found = true;
						}
					}

					if (found) {
						continue;
					}

					// right left
					if (coords[i].x == coords[j].x + scanSize && coords[i].y == coords[j].y) { // right
						count += 1;
						c.x = coords[j].x + scanSize;
						c.y = coords[j].y;
						checked.push_back(c);
						continue;
					}
					if (coords[i].x == coords[j].x - scanSize && coords[i].y == coords[j].y) { // left
						count += 1;
						c.x = coords[j].x - scanSize;
						c.y = coords[j].y;
						checked.push_back(c);
						continue;
					}

					// up down
					if (coords[i].x == coords[j].x && coords[i].y == coords[j].y + scanSize) { // down
						count += 1;
						c.x = coords[j].x;
						c.y = coords[j].y + scanSize;
						checked.push_back(c);
						continue;
					}
					if (coords[i].x == coords[j].x && coords[i].y == coords[j].y - scanSize) { // up
						count += 1;
						c.x = coords[j].x;
						c.y = coords[j].y - scanSize;
						checked.push_back(c);
						continue;
					}

					// diagonals
					if (coords[i].x == coords[j].x + scanSize && coords[i].y == coords[j].y + scanSize) { // bottom right
						count += 1;
						c.x = coords[j].x + scanSize;
						c.y = coords[j].y + scanSize;
						checked.push_back(c);
						continue;
					}
					if (coords[i].x == coords[j].x + scanSize && coords[i].y == coords[j].y - scanSize) { // top right
						count += 1;
						c.x = coords[j].x + scanSize;
						c.y = coords[j].y - scanSize;
						checked.push_back(c);
						continue;
					}
					if (coords[i].x == coords[j].x - scanSize && coords[i].y == coords[j].y + scanSize) { // bottom left
						count += 1;
						c.x = coords[j].x - scanSize;
						c.y = coords[j].y + scanSize;
						checked.push_back(c);
						continue;
					}
					if (coords[i].x == coords[j].x - scanSize && coords[i].y == coords[j].y - scanSize) { // top left
						count += 1;
						c.x = coords[j].x - scanSize;
						c.y = coords[j].y - scanSize;
						checked.push_back(c);
						continue;
					}
				}
			}

			std::cout << "Found " << count << " clusters." << std::endl;

			
		}

		// end cluster logic

		for (int i = 0; i < coords.size(); i++) {
			int x = coords[i].x * (imageWidth / windowWidth);
			int y = coords[i].y * (imageHeight / windowHeight);
			cluster.setPosition(sf::Vector2f(x, y));
			window.draw(cluster);
		}

		//text.setString(std::to_string(clusters[i].value));
		//text.setPosition(x, y);
		//window.draw(text);


		window.display();
		
	}

	return 0;
}