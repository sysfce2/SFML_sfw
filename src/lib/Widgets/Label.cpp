#include "sfw/Widgets/Label.hpp"
#include "sfw/Theme.hpp"
#include "sfw/util/shim/sfml.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

namespace sfw
{

Label::Label(const std::string& text)
{
    onThemeChanged(); //!!Calling it this way is a temp. kludge (for DRY). Also: it has to happen before the rest of the init.
    setFocusable(false);
    setText(text);
}


Label* Label::setText(const std::string& text)
{
    m_text.setString(/*sfw::*/stdstring_to_SFMLString(text));
    recomputeGeometry();
    return this;
}

std::string Label::getText() const
{
    return /*sfw::*/SFMLString_to_stdstring(m_text.getString());
}


Label* Label::setFillColor(const sf::Color& color)
{
    m_text.setFillColor(color);
    return this;
}

const sf::Color& Label::getFillColor() const
{
    return m_text.getFillColor();
}


Label* Label::setTextSize(size_t size)
{
    m_text.setCharacterSize((unsigned)size);
    recomputeGeometry();
    return this;
}

size_t Label::getTextSize() const
{
    return m_text.getCharacterSize();
}


Label* Label::setStyle(sf::Text::Style style)
{
    m_text.setStyle(style);
    return this;
}


void Label::draw(const gfx::RenderContext& ctx) const
{
    auto sfml_renderstates = ctx.props;
    sfml_renderstates.transform *= getTransform();
    ctx.target.draw(m_text, sfml_renderstates);
}


void Label::onThemeChanged()
{
    m_text.setFont(Theme::getFont());
    m_text.setPosition({Theme::PADDING, Theme::PADDING}); //!!  ... + m_text.getLocalBounds().top});
                                                          //!! The "canonical" SFML offest correction would
                                                          //!! make the positioning inconsistent: some text
                                                          //!! would then sit on the baseline, some won't etc.!
    m_text.setFillColor(Theme::click.textColor);
    m_text.setCharacterSize((unsigned)Theme::textSize);
    recomputeGeometry();
}


void Label::recomputeGeometry()
{
    auto bounds = m_text.getLocalBounds();
    Widget::setSize(
        bounds.width + Theme::PADDING * 2 + bounds.left,
        bounds.height + Theme::PADDING * 2 + bounds.top
    );
}

} // namespace
