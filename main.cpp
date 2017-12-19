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
void findClusters(std::vector<Coord> &coords, int *pixels, double density, int scanSize, int checkSize, int width, int height) {
	Coord c;
	int dx;
	int dy;
	int v;
	int sum = 0;
	int mean;
	int count = 0;
	int scanPixelCount = scanSize * scanSize;
	int threshold = scanPixelCount * density; // 0.4
	bool flag = false; // means ignore this cluster
	int pixelsScanned = 0;

	sum = 0;
	int pixelCount = sizeof(pixels) / sizeof(*pixels);
	for (int i = 0; i < pixelCount; i++) {
		sum += pixels[i];
	}

	int colourThreshold = sum / pixelCount; // the average pixel intensity

	// for every vertical scan region
	for (int y = 0; y < height - scanSize / 2; y += scanSize / 2) {

		// for every horizontal scan region
		for (int x = 0; x < width - scanSize / 2; x += scanSize / 2) {
			flag = true;

			if (x < width - scanSize && y < height - scanSize) {
				count = 0;
				sum = 0;
				for (int j = y; j < y + scanSize; j++) {
					for (int i = x; i < x + scanSize; i++) {
						v = pixels[(j * width) + i];
						if (v > colourThreshold) { // 50
							count += 1; // count neighbouring pixels
							sum += v;
						}
					}
				}

				// if cell count is over threshold and average pixel intensity is greater than the colour threshold
				if (count > threshold) {// && sum / count > colourThreshold) { 
					flag = false;
				}

				bool sameCluster = false;

				if (!flag) {

					// check around this point for other prviously marked points
					for (int i = 0; i < coords.size(); i++) {
						dy = abs(y - coords[i].y);
						dx = abs(x - coords[i].x);
						int dist = sqrt((dx * dx) + (dy * dy));

						if (dist < checkSize) {
							sameCluster = true;
							continue;
						}

						// if the distance is large then we can just ignore this cluster
						if (dist > checkSize) {
							continue;
						}

						// scan pixels around this cluster to see if they have already been stored
						for (int j = y - checkSize; j < y + checkSize; j++) {

							for (int k = x - checkSize; k < x + checkSize; k++) {

								if (k == coords[i].x && j == coords[i].y) {
									sameCluster = true;
								}
							}
						}
					}

					if (!sameCluster) {
						c.x = x;
						c.y = y;
						c.value = sum / count; // average intensity
						coords.push_back(c);
						flag = true;
					}
				}
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
	int scanSize = imageWidth / 80;
	int checkSize = scanSize * 4;

	sf::RectangleShape cluster(sf::Vector2f(scanSize, scanSize));
	cluster.setOutlineColor(sf::Color::Red);
	cluster.setOutlineThickness(2);
	cluster.setFillColor(sf::Color::Transparent);

	sf::RectangleShape pixel(sf::Vector2f(1, 1));

	while (window.isOpen()) {

		sf::Event event;
		while (window.pollEvent(event)) {

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		window.clear(sf::Color::White);

		if (!processed) {
			int i = 0;

			for (int y = 0; y < imageHeight; y++) {
				for (int x = 0; x < imageWidth; x++) {
					pixels[i] = image.getPixel(x, y).b;
					i += 1;
				}
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
			findClusters(clusters, pixels, 0.5, scanSize, checkSize, imageWidth, imageHeight);
			std::cout << "Found " << clusters.size() << " clusters." << std::endl;
			processed = true;
		}

		for (int i = 0; i < clusters.size(); i++) {
			int x = clusters[i].x * (imageWidth / windowWidth);
			int y = clusters[i].y * (imageHeight / windowHeight);
			cluster.setPosition(sf::Vector2f(x, y));
			window.draw(cluster);
			text.setString(std::to_string(clusters[i].value));
			text.setPosition(x, y);
			window.draw(text);
		}

		window.display();
	}

	return 0;
}