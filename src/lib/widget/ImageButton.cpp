#include "sfw/widget/ImageButton.hpp"
#include "sfw/Theme.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <cmath>
	using std::round;

namespace sfw
{

ImageButton::ImageButton(const sf::Texture& texture, const std::string& label):
	m_background(texture), // no default sf::Sprite ctor
	m_pressed(false)
{
	setTexture(texture);
	m_text.setFont(Theme::getFont());
	m_text.setCharacterSize((unsigned)Theme::textSize);

	setText(label);
}


ImageButton* ImageButton::setTexture(const sf::Texture& texture)
{
	int width = texture.getSize().x;
	int height = texture.getSize().y / 3; // default, hover, focus

	m_background.setTexture(texture);
	m_background.setTextureRect(sf::IntRect({0, 0}, {width, height}));

	Widget::setSize((float)width, (float)height);
	return this;
}


ImageButton* ImageButton::setText(const std::string& label)
{
	m_text.set(label);
	centerText();
	return this;
}

std::string ImageButton::getText() const
{
	return m_text.get();
}

/*
ImageButton* ImageButton::setText(const sf::String& text)
{
	m_text.setString(text);
	centerText();
	return this;
}

const sf::String& ImageButton::getText() const
{
	return m_text.getString();
}
*/

ImageButton* ImageButton::setSize(sf::Vector2f size)
{
	m_background.setScale({size.x / m_background.getTexture().getSize().x,
	                       size.y / m_background.getTexture().getSize().y * 3}); // see the ctor for that 3! ;)
	Widget::setSize(size);
	return this;
}

ImageButton* ImageButton::setFont(const sf::Font& font)
{
	m_text.setFont(font);
	centerText();
	return this;
}


const sf::Font& ImageButton::getFont() const
{
	return m_text.getFont();
}


ImageButton* ImageButton::setTextSize(size_t size)
{
	m_text.setCharacterSize((unsigned)size);
	centerText();
	return this;
}


ImageButton* ImageButton::setTextStyle(sf::Text::Style style)
{
	m_text.setStyle(style);
	return this;
}


ImageButton* ImageButton::setTextColor(sf::Color color)
{
	m_text.setFillColor(color);
	return this;
}


void ImageButton::draw(const gfx::RenderContext& ctx) const
{
	auto sfml_renderstates = ctx.props;
	sfml_renderstates.transform *= getTransform();
	ctx.target.draw(m_background, sfml_renderstates);
	sfml_renderstates.transform *= m_background.getTransform(); // Follow the scaling (etc.) of the image!
	m_text.draw({ctx.target, sfml_renderstates});
}


// Callbacks -------------------------------------------------------------------

void ImageButton::onActivationChanged(ActivationState state)
{
	sf::Vector2i size(m_background.getTexture().getSize().x,
	                  m_background.getTexture().getSize().y / 3);
	switch (state)
	{
	case ActivationState::Default:
		m_background.setTextureRect(sf::IntRect({0, 0}, {size.x, size.y}));
		break;
	case ActivationState::Hovered:
		m_background.setTextureRect(sf::IntRect({0, size.y}, {size.x, size.y}));
		break;
	case ActivationState::Pressed:
	case ActivationState::Focused:
		m_background.setTextureRect(sf::IntRect({0, size.y * 2}, {size.x, size.y}));
		break;
	case ActivationState::Disabled:
		break;
	}
}


void ImageButton::onMouseMoved(float x, float y)
{
	if (focused())
	{
		if (contains({x, y}) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			press();
		else
			release();
	}
}


void ImageButton::onMousePressed(float, float)
{
	press();
}


void ImageButton::onMouseReleased(float x, float y)
{
	release();
	if (contains({x, y}))
	{
		onUpdated();
	}
}


void ImageButton::onKeyPressed(const sf::Event::KeyChanged& key)
{
	if (key.code == sf::Keyboard::Key::Enter)
	{
		press();
		onUpdated();
	}
}


void ImageButton::onKeyReleased(const sf::Event::KeyChanged& key)
{
	if (key.code == sf::Keyboard::Key::Enter)
	{
		release();
	}
}


void ImageButton::press()
{
	if (!m_pressed)
	{
		m_pressed = true;
		m_text.move({0, 1});
	}
}

void ImageButton::release()
{
	if (m_pressed)
	{
		m_pressed = false;
		m_text.move({0, -1});
	}
}


void ImageButton::centerText()
{
	auto [boxwidth, boxheight] = getSize();
	sf::FloatRect t = m_text.getLocalBounds();
	m_text.setOrigin({t.left + round(t.width / 2.f), t.top + round(t.height / 2.f)});
	m_text.setPosition({boxwidth / 2, boxheight / 2});
}

} // namespace