#include "sfw/Theme.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace sfw
{

template <class T>
ItemBox<T>::ItemBox(Box::Type type):
    Box(type)
{
    applyState(ActivationState::Default);
}

template <class T>
ItemBox<T>::ItemBox(const T& item, Box::Type type):
    Box(type),
    m_item(item)
{
    applyState(ActivationState::Default);
}


template <class T>
void ItemBox<T>::setItemColor(sf::Color color)
{
    m_itemColor = color;
    m_item.setFillColor(m_itemColor.value());
}

template <class T>
void ItemBox<T>::setFillColor(sf::Color color)
{
    Box::setFillColor(color);
}

template <class T>
void ItemBox<T>::setTintColor(sf::Color color)
{
    m_tintColor = color;
}


template <class T>
void ItemBox<T>::applyState(ActivationState state)
{
    Box::applyState(state);
    if (m_itemColor)
    {
        m_item.setFillColor(m_itemColor.value());
    }
    else switch (state)
    {
    case ActivationState::Default:
        m_item.setFillColor(m_type == Click ? Theme::click.textColor : Theme::input.textColor);
        break;
    case ActivationState::Hovered:
        m_item.setFillColor(m_type == Click ? Theme::click.textColorHover : Theme::input.textColorHover);
        break;
    case ActivationState::Pressed:
    case ActivationState::Focused:
        m_item.setFillColor(m_type == Click ? Theme::click.textColorFocus : Theme::input.textColorFocus);
        break;
    case ActivationState::Disabled:
        m_item.setFillColor(m_type == Click ? Theme::click.textColorDisabled : Theme::input.textColorDisabled);
        break;
    }
}

template <class T>
void ItemBox<T>::draw(sf::RenderTarget& target, const sf::RenderStates& states) const
{
    Box::draw(target, states);
    target.draw(m_item, states);
    // "Tint" the box *after* the item has been drawn! (So, alpha is expected to have been set accordingly!)
    if (m_tintColor)
    {
/*
        sf::RectangleShape r(sf::Vector2f(getSize().x - 2 * (float)Theme::borderSize,
                                          getSize().y - 2 * (float)Theme::borderSize));
        r.setPosition(getPosition()); r.move({(float)Theme::borderSize, (float)Theme::borderSize});
*/
        sf::RectangleShape r(sf::Vector2f(getSize().x, getSize().y));
        r.setPosition(getPosition());

        r.setFillColor(m_tintColor.value());
        r.setOutlineThickness(0);
        target.draw(r, states);
    }
}

template <class T>
void ItemBox<T>::onPress()
{
    m_item.move({0.f, 1.f});
}

template <class T>
void ItemBox<T>::onRelease()
{
    m_item.move({0.f, -1.f});
}

} // namespace
