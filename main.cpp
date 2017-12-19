#include <iostream>
#include <SFML\Graphics.hpp>
#include "constants.h"

struct Coord {
	int x;
	int y;
	int value;
};

namespace test {

} // namespace

// scans the whole image for clusters and stores them in the coords vector
void findClusters(std::vector<Coord> &coords, int *pixels, double tolerance, int width, int height) {
	Coord c;
	int v;
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

	int threshold = (sum / count) * (1 / (1 / tolerance)); 

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
				c.x = x;
				c.y = y;
				c.value = v; // average intensity
				coords.push_back(c);
			}
		}
	}
}

int main() {
	sf::Image image;
	sf::Font font;

	if (!image.loadFromFile("test.png")) {
		return -1;
	}

	if (!font.loadFromFile("font.ttf")) {
		return -1;
	}
	
	const int imageWidth = image.getSize().x;
	const int imageHeight = image.getSize().y;

	sf::RenderWindow window(sf::VideoMode(imageWidth, imageHeight), "Application");
	window.setFramerateLimit(30);

	sf::Text text;
	text.setFont(font);
	text.setFillColor(sf::Color::Black);
	text.setCharacterSize(40);

	std::vector<Coord> clusters;
	int *pixels = new int[imageWidth * imageHeight];
	bool processed = false;
	int scanCount = 100;
	int scanSize = imageWidth / 100;
	//int checkSize = 0;

	sf::RectangleShape cluster(sf::Vector2f(scanSize - 1, scanSize - 1));
	cluster.setFillColor(sf::Color::Red);

	sf::RectangleShape pixel(sf::Vector2f(1, 1));

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
			findClusters(clusters, pixels, 0.85, imageWidth, imageHeight);
			std::cout << "Found " << clusters.size() << " clusters." << std::endl;
			processed = true;
		}

		for (int i = 0; i < clusters.size(); i++) {
			int x = clusters[i].x * (imageWidth / windowWidth);
			int y = clusters[i].y * (imageHeight / windowHeight);
			cluster.setPosition(sf::Vector2f(x, y));
			window.draw(cluster);
			//text.setString(std::to_string(clusters[i].value));
			//text.setPosition(x, y);
			//window.draw(text);
		}

		window.display();
	}

	return 0;
}