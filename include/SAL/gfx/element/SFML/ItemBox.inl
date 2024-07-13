#include "sfw/Theme.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace sfw
{

template <Shape T>
ItemBox<T>::ItemBox(Box::Type type):
	Box(type)
{
	applyState(ActivationState::Default);
}

template <Shape T>
ItemBox<T>::ItemBox(const T& item, Box::Type type):
	Box(type),
	m_item(item)
{
	applyState(ActivationState::Default);
}


template <Shape T>
void ItemBox<T>::setItemColor(sf::Color color)
{
	m_itemColor = color;
	m_item.setFillColor(m_itemColor.value());
}

template <Shape T>
void ItemBox<T>::setFillColor(sf::Color color)
{
	Box::setFillColor(color);
}

template <Shape T>
void ItemBox<T>::setTintColor(sf::Color color)
{
	m_tintColor = color;
}


template <Shape T>
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

template <Shape T>
void ItemBox<T>::draw(const gfx::RenderContext& ctx) const //override
{
	Box::draw(ctx); //! Qualified (with the base) to avoid infinite recursion...
	m_item.draw(ctx);
	// Tint the box *after* the item has been drawn! (So, alpha is expected to have been set accordingly!)
	if (m_tintColor)
	{
/*
		sf::RectangleShape r(sf::Vector2f(getSize().x - 2 * (float)Theme::borderSize,
		                                  getSize().y - 2 * (float)Theme::borderSize));
		r.setPosition(getPosition()); r.move({(float)Theme::borderSize, (float)Theme::borderSize});
*/
		auto r = sf::RectangleShape(getSize()); r.setPosition(getPosition());
		//!! Should be:
		//!!auto r = sf::RectangleShape( {getPosition(), getSize()} ); // auto fRect -> sf::Rect<float>
		//!! or:
		//!!auto r = sf::RectangleShape{ getPosition(), getSize() };
		//!! but SFML's RectangleShape still doesn't seem to have "rectangular" ctors!... :-o
		r.setFillColor(m_tintColor.value());
		r.setOutlineThickness(0);
		ctx.target.draw(r, ctx.props);
	}
}

template <Shape T>
void ItemBox<T>::onPress()
{
	m_item.move({0.f, 1.f});
}

template <Shape T>
void ItemBox<T>::onRelease()
{
	m_item.move({0.f, -1.f});
}

} // namespace
