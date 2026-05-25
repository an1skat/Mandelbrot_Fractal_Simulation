// Підключення всіх потрібних пакетів
#include <SFML/Graphics.hpp>
#include <sstream>

// Оголошуємо namespace. Це більше правило класичного коду, а також міні отпимізація для компілятора, щоб він знав, що ці функції використовуют
namespace {
	const sf::Vector2u WindowSize(1600, 900);

	constexpr double InitialRm = -2.0;
	constexpr double InitialRM = 1.0;
	constexpr double InitialIm = -1.2;
	constexpr double InitialIM = 1.2;

	constexpr int InitialMaxIt = 80;
	constexpr int MinIt = 10;
	constexpr int ItStep = 5;

	constexpr double PanSpeed = 0.65;
	constexpr double ZoomSpeed = 1.2;	

	constexpr float ItRepeatDelay = 0.05f;

	struct FractalView {
		double Rm = InitialRm;
		double RM = InitialRM;
		double Im = InitialIm;
		double IM = InitialIM;

		double width() const {
			return RM - Rm;
		}
		double height() const {
			return IM - Im;
		}
		double centerR() const {
			return (Rm + RM) / 2.0;
		}
		double centerI() const {
			return (Im + IM) / 2.0;
		}
		double zoom() const {
			return (InitialRM - InitialRm) / width();
		}
		void reset() {
			Rm = InitialRm;
			RM = InitialRM;
			Im = InitialIm;
			IM = InitialIM;
		}

		void pan(double RDelta, double IDelta) {
			Rm += RDelta;
			RM += RDelta;
			Im += IDelta;
			IM += IDelta;
		}

		void scaleAroundCenter(double factor) {
			const double nextWidth = width() * factor;
			const double nextHeight = height() * factor;
			const double RCenter = centerR();
			const double ICenter = centerI();

			Rm = RCenter - nextWidth / 2.0;
			RM = RCenter + nextWidth / 2.0;
			Im = ICenter - nextHeight / 2.0;
			IM = ICenter + nextHeight / 2.0;
		}
	};

	void applyDiscreteInput(
		const sf::Event::KeyPressed& key,
		sf::RenderWindow& window,
		FractalView& view) {
			switch (key.scancode) {
				case sf::Keyboard::Scancode::Escape:
					window.close();
					break;
				case sf::Keyboard::Scancode::R:
					view.reset();
					break;
				
				default:
					break;
			}
	}
	void handleEvents(
		sf::RenderWindow& window,
		FractalView& view ) {
			while (const std::optional event = window.pollEvent()) {
				if (event->is<sf::Event::Closed>()){
					window.close();
					break;
				}
				if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
					applyDiscreteInput(*key, window, view);
				}
			}
		}

	double keyAxis(
	sf::Keyboard::Scancode negative,
	sf::Keyboard::Scancode positive) {
		const double negativeValue = sf::Keyboard::isKeyPressed(negative) ? -1.0 : 0.0;
		const double positiveValue = sf::Keyboard::isKeyPressed(positive) ? 1.0 : 0.0;
		return negativeValue + positiveValue;
	}

	void updateIt(int& maxIt, float deltaSec) {
		static float timer = 0.0f;
		timer += deltaSec;
		if (timer < ItRepeatDelay) return;

		timer = 0.0f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LBracket)) {
			maxIt = std::max(MinIt, maxIt - ItStep);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::RBracket)) {
			maxIt += ItStep;
		}
	}

	void applyRealtimeInput(FractalView& view, int& maxIt, float deltaSec) {
		const double horizontal = keyAxis(
			sf::Keyboard::Scancode::A,
			sf::Keyboard::Scancode::D
		);
		const double vertical = keyAxis(
			sf::Keyboard::Scancode::W,
			sf::Keyboard::Scancode::S
		);

		if (horizontal != 0.0 || vertical != 0.0) {
			view.pan(
				horizontal * view.width() * PanSpeed * deltaSec,
				vertical * view.height() * PanSpeed * deltaSec
			);
		}

		const double zoomDir = keyAxis(
			sf::Keyboard::Scancode::Up,
			sf::Keyboard::Scancode::Down
		);

		if (zoomDir != 0.0) {
			view.scaleAroundCenter(std::exp(ZoomSpeed * zoomDir * deltaSec));
		}
		updateIt(maxIt, deltaSec);
	}

	std::string makeInfoString(float fps, int maxIt, double zoom) {
		std::ostringstream stream;

		stream<<"FPS: "<<static_cast<int>(fps)<<"\n Iterations: "<<maxIt<<"\nZoom: x";
		if (zoom < 10.0) {
			stream<<std::fixed<<std::setprecision(2)<<zoom;
		} else {
			stream<<std::fixed<<std::setprecision(0)<<zoom;
		}

		return stream.str();
	}

	bool updateFps(sf::Clock& clock, int& frameCount, float& currFps) {
		frameCount++;
		if (clock.getElapsedTime().asSeconds() < 0.5f) {
			return false;
		}

		currFps = static_cast<float>(frameCount) / clock.restart().asSeconds();

		frameCount = 0;
		return true;
	}

	void applyShaderUniforms(sf::Shader& shader, const FractalView& view, int maxIt) {
		shader.setUniform("resolution", sf::Glsl::Vec2(WindowSize));
		shader.setUniform("Rm", static_cast<float>(view.Rm));
		shader.setUniform("RM", static_cast<float>(view.RM));
		shader.setUniform("Im", static_cast<float>(view.Im));
		shader.setUniform("IM", static_cast<float>(view.IM));
		shader.setUniform("maxIt", maxIt);
	}
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(WindowSize), "Mandelbrot Fractal Simulation");
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(false);
	window.setKeyRepeatEnabled(false);

	sf::Shader shader;

	if (!shader.loadFromFile("mandelbrot.frag", sf::Shader::Type::Fragment)) return -1;

	sf::RectangleShape screenRect;
	screenRect.setSize(sf::Vector2f(
		static_cast<float>(WindowSize.x),
		static_cast<float>(WindowSize.y)
	));

	FractalView view;
	int maxIt = InitialMaxIt;

	sf::Font font;
	const bool fontLoaded = font.openFromFile("font.ttf");

	sf::Text infoText(font);

	infoText.setCharacterSize(18);
  infoText.setFillColor(sf::Color::White);
	infoText.setPosition({10.f, 10.f});

	sf::Clock frameClock, fpsClock;
	int frameCount = 0;
	float currFps = 0.f;

	if (fontLoaded) {
		infoText.setString(makeInfoString(currFps, maxIt, view.zoom()));
	}

	while (window.isOpen()) {
		handleEvents(window, view);

		const float deltaSec = std::min(frameClock.restart().asSeconds(), 0.05f);

		applyRealtimeInput(view, maxIt, deltaSec);

		if (fontLoaded && updateFps(fpsClock, frameCount, currFps)) {
			infoText.setString(makeInfoString(currFps, maxIt, view.zoom()));
		}

		applyShaderUniforms(shader, view, maxIt);

		window.clear();

		sf::RenderStates states;
		states.shader = &shader;

		window.draw(screenRect, states);

		if (fontLoaded) window.draw(infoText);

		window.display();
	}

	return 0;
}