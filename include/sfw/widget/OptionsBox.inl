#include "sfw/Theme.hpp"

#include "SAL/sfml.hpp" // std::string[_view] <-> sf::String conv. (Should be "interfaced" away!...)
#include "SAL/util/diagnostics.hpp"

#include <algorithm>
	using std::max;


namespace sfw
{

template <class T> OptionsBox<T>::OptionsBox():
	m_currentIndex(0), // Used to start with the last: (size_t)-1, but #359...
	m_box(Box::Input),
	m_arrowLeft(Arrow(Arrow::Direction::Left)),
	m_arrowRight(Arrow(Arrow::Direction::Right))
{
	onThemeChanged();
}

template <class T> OptionsBox<T>::OptionsBox(std::function<void(OptionsBox<T>*)> callback):
	OptionsBox()
{
	this->setCallback(callback); //! See comment at the class def., why this->...
}


template <class T> auto OptionsBox<T>::add(std::string_view label, const T& value)
{
	m_items.push_back(Item(label, value));

	m_box.item().set(label);
	// Check if the box needs to be resized
	float width = m_box.item().size().x() + Theme::getBoxHeight() * 2 + Theme::PADDING * 2; //!! Height used directly in a width calc...
	if (width > this->getSize().x()) //! See comment at the class def., why this->...
	{
		m_box.setSize(width, (float)Theme::getBoxHeight());
		m_arrowRight.setPosition(width - (float)Theme::getBoxHeight(), 0.f);
		m_arrowRight.centerItem(m_arrowRight.item());
		this->setSize(m_box.getSize()); //! See comment at the class def., why this->...
	}

// Don't (as per #359):
//	update_selection(m_items.size() - 1);
// But update_selection() is still needed to prepare the looks, so:
	update_selection(m_currentIndex);
	return this;
}


template <class T> auto OptionsBox<T>::assign(std::string_view label, const T& value)
{
	for (size_t i = 0; i < m_items.size(); ++i)
	{
		if (label == m_items[i].label)
		{
			m_items[i].value = value;
			break;
		}
	}
	return this;
}


template <class T> auto OptionsBox<T>::set(size_t index)
{
	if (index != m_currentIndex)
	{
		this->setChanged(); //! this-> required here due to C++ cringe...
		update_selection(index);
	}
	return this;
}

template <class T> auto OptionsBox<T>::set(std::string_view label)
{
	for (size_t i = 0; i < m_items.size(); ++i)
	{
		if (label == m_items[i].label)
		{
			return set(i);
		}
	}
	return this;
}

template <class T> auto OptionsBox<T>::set(const T& value)
{
	for (size_t i = 0; i < m_items.size(); ++i)
	{
		if (value == m_items[i].value)
		{
			return set(i);
		}
	}
	return this;
}

template <class T> auto OptionsBox<T>::select(size_t index)
{
	return this->update(index);
}

template <class T> auto OptionsBox<T>::select(std::string_view label)
{
	return this->update(label);
}

template <class T> auto OptionsBox<T>::selectNext()
{
	return m_items.size() < 1 ? this
		: select(m_currentIndex == m_items.size() - 1 ? 0 : m_currentIndex + 1);
}

template <class T> auto OptionsBox<T>::selectPrevious()
{
	return m_items.size() < 1 ? this
		: select(m_currentIndex == 0 ? m_items.size() - 1 : m_currentIndex - 1);
}

template <class T> auto OptionsBox<T>::selectFirst()
{
	return m_items.size() < 1 ? this : select(0);
}

template <class T> auto OptionsBox<T>::selectLast()
{
	return m_items.size() < 1 ? this : select(m_items.size() - 1);
}


template <class T> const T& OptionsBox<T>::get() const
{
	return m_items[m_currentIndex].value;
}

template <class T> T& OptionsBox<T>::get()
{
	return m_items[m_currentIndex].value;
}

template <class T> size_t OptionsBox<T>::currentIndex() const
{
	return m_currentIndex;
}

template <class T> const std::string& OptionsBox<T>::currentLabel() const
{
	return m_items[m_currentIndex].label;
}


template <class T> OptionsBox<T>* OptionsBox<T>::setTextColor(Color color)
{
	m_box.setItemColor(color);
	return this;
}

template <class T> OptionsBox<T>* OptionsBox<T>::setFillColor(Color color)
{
	m_box.setFillColor(color);
	m_arrowLeft.setTintColor(color);
	m_arrowRight.setTintColor(color);
	return this;
}


template <class T> auto OptionsBox<T>::update_selection(size_t index)
{
	//! No check for index != m_currentIndex, because this may also be called from
	//! onThemeChanged, where "updating" would just mean restoring the current label
	//! and re-centering it, without actually changing the current index!
	if (index < m_items.size())
	{
		m_currentIndex = index;
		static_assert(std::is_convertible_v<decltype(m_box.item()), gfx::Text>, "The label can only be gfx::Text currently!");
		m_box.item().set(m_items[index].label);
		m_box.item().center_in(m_box.getRect());
	}
	return this;
}


template <class T> void OptionsBox<T>::update_arrow_pressed_state(ItemBox<Arrow>& arrow, float x, float y)
{
	if (arrow.contains(x, y))
	{
		if (this->getActivationState() == ActivationState::Pressed) // See comment at the class def., why this->...
			arrow.press();
		else
			arrow.applyState(ActivationState::Hovered);
	}
	else
	{
		arrow.applyState(this->focused() ? ActivationState::Focused : ActivationState::Default); // See comment at the class def., why this->...
	}
}


template <class T> void OptionsBox<T>::draw(const gfx::RenderContext& ctx) const
{
	auto lctx = ctx;
	lctx.props.transform *= this->getTransform(); // See comment at the class def., why this->...
	m_box.draw(lctx);
	m_arrowLeft.draw(lctx);
	m_arrowRight.draw(lctx);
}


// callbacks -------------------------------------------------------------------

template <class T> void OptionsBox<T>::onThemeChanged()
{
	static_assert(std::is_convertible_v<decltype(m_box.item()), gfx::Text>); // Just in case I ever change it... ;)
	m_box.item().font(Theme::getFont());
	m_box.item().font_size((unsigned)Theme::fontSize);

	// Update width to accomodate the widest element
	auto width = (float)Theme::minWidgetWidth;
	for (size_t i = 0; i < m_items.size(); ++i)
	{
		m_box.item().set(m_items[i].label);
		width = max(width, m_box.item().size().x() + Theme::getBoxHeight() * 2 + Theme::PADDING * 2);
	}
	m_box.setSize(width, (float)Theme::getBoxHeight());
	this->setSize(m_box.getSize()); // See comment at the class def., why this->...

	//! Reset the current selection rolled all over in the loop above.
	//! This will also re-center it, so the widget had to have been resized first!
	update_selection(m_currentIndex);

	// Left arrow
	m_arrowLeft.setSize(Theme::getBoxHeight(), Theme::getBoxHeight());
	m_arrowLeft.centerItem(m_arrowLeft.item());

	// Right arrow
	m_arrowRight.setSize(Theme::getBoxHeight(), Theme::getBoxHeight());
	m_arrowRight.setPosition(m_box.getSize().x() - Theme::getBoxHeight(), 0);
	//!!?? WOW! Doing the same in reverse would make it fall apart spectacularly!! :-ooo
	//!!m_arrowRight.setPosition(m_box.getSize().x - Theme::getBoxHeight(), 0);
	//!!m_arrowRight.setSize(Theme::getBoxHeight(), Theme::getBoxHeight());
	m_arrowRight.centerItem(m_arrowRight.item());
}


template <class T> void OptionsBox<T>::onActivationChanged(ActivationState state)
{
	if (state == ActivationState::Default || state == ActivationState::Focused) // Hovering is handled by onMouseMoved!
	{
		m_arrowLeft.applyState(state);
		m_arrowRight.applyState(state);
		m_box.applyState(state);
	}
}


template <class T> void OptionsBox<T>::onMouseMoved(float x, float y)
{
	update_arrow_pressed_state(m_arrowLeft, x, y);
	update_arrow_pressed_state(m_arrowRight, x, y);
}


template <class T> void OptionsBox<T>::onMousePressed(float x, float y)
{
	if (m_arrowLeft.contains(x, y))
		m_arrowLeft.press();

	else if (m_arrowRight.contains(x, y))
		m_arrowRight.press();
}


template <class T> void OptionsBox<T>::onMouseReleased(float x, float y)
{
	if (m_arrowLeft.contains(x, y))
	{
		m_arrowLeft.release();
		selectPrevious();
	}
	else if (m_arrowRight.contains(x, y))
	{
		m_arrowRight.release();
		selectNext();
	}
}


template <class T> void OptionsBox<T>::onMouseWheelMoved(int delta)
{
	if (delta < 0) selectNext(); else selectPrevious();
}


template <class T> void OptionsBox<T>::onKeyPressed(const event::KeyCombination& key)
{
	switch (key.code) //!!XLAT below!
	{
	case unsigned(sf::Keyboard::Key::Left):
	case unsigned(sf::Keyboard::Key::Up):
		selectPrevious();
		m_arrowLeft.press();
		break;
	case unsigned(sf::Keyboard::Key::Right):
	case unsigned(sf::Keyboard::Key::Down):
		selectNext();
		m_arrowRight.press();
		break;
	case unsigned(sf::Keyboard::Key::Home):
	case unsigned(sf::Keyboard::Key::PageUp):
		selectFirst();
		break;
	case unsigned(sf::Keyboard::Key::End):
	case unsigned(sf::Keyboard::Key::PageDown):
		selectLast();
		break;
	default: ; // (Just for GCC to shut up...)
	}
}

template <class T> void OptionsBox<T>::onKeyReleased(const event::KeyCombination& key)
{
	switch (key.code) //!!XLAT below!
	{
	case unsigned(sf::Keyboard::Key::Left):
	case unsigned(sf::Keyboard::Key::Up):
		m_arrowLeft.release();
		// Without this the focus rect would be lost on the arrow (#137):
		update_arrow_pressed_state(m_arrowLeft, -1, -1); // -1,-1 to avoid the "hover" state
		break;
	case unsigned(sf::Keyboard::Key::Right):
	case unsigned(sf::Keyboard::Key::Down):
		m_arrowRight.release();
		// Without this the focus rect would be lost on the arrow (#137):
		update_arrow_pressed_state(m_arrowRight, -1, -1); // -1,-1 to avoid the "hover" state
		break;
	default: ; // (Just for GCC to shut up...)
	}
}


template <class T> OptionsBox<T>::Item::Item(std::string_view text, const T& val):
	label(text),
	value(val)
{
}


} // namespace sfw
