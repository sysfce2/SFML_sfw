#ifndef _SFW_OPTIONSBOX_HPP_
#define _SFW_OPTIONSBOX_HPP_

#include "sfw/InputWidget.hpp"
#include "sfw/Gfx/Elements/Text.hpp"
#include "sfw/Gfx/Elements/Arrow.hpp"
#include "sfw/Gfx/Elements/ItemBox.hpp"

#include <string>

namespace sfw
{

/*===========================================================================
  Widget for selecting an item from a list. It can only display the currently
  selected item, so it's essentially a collapsed list box...

  The list order will match the order of the add() calls.
  Item indexes starts with 0.

  The update notification callback is triggered when the item selection changes.
 ===========================================================================*/

template <class T>
class OptionsBox: public InputWidget<OptionsBox<T>>
	//! Alas, there's an obscure C++ restriction for (ADL) name lookup that prevents
	//! members of Widget -- which is the base of the CRTPed InputWidget class --
	//! from being found by the compiler! "Fortunately", prepending this-> to those
	//! still works...
{
public:
	OptionsBox();
	OptionsBox(std::function<void(OptionsBox<T>*)> callback);

	// -------- Setup...

	// Append new item to the list
	auto add(const std::string& label, const T& value);

	// Change the value of an existing item (i.e. key = value)
	auto assign(const std::string& label, const T& value);

	// Change the current item
	auto set(size_t index);
	auto set(const std::string& label);
	auto set(const T& value); // Would call T::op==!

	OptionsBox<T>* setTextColor(const sf::Color& color);
	OptionsBox<T>* setFillColor(const sf::Color& color);

	// -------- Actions...

	// Select item (by index or label)
	// (Synonym to update(...), which is pretty awkward here.)
	auto select(size_t index);             
	auto select(const std::string& label);
	auto select(const T& value);

	// Actions for Up/Down, Home/End...
	auto selectNext();
	auto selectPrevious();
	auto selectFirst();
	auto selectLast();

	// -------- Queries...

	// Accessing the selected item
	const T& get() const;     // Const ref. instead of val., as T can be big!
	      T& get();           // Non-const, for in-place modification
	// Just a synonym (for the symmetry with 'select'...):
	const T& current() const  { return get(); }
	      T& current()        { return get(); }

	size_t currentIndex() const;
	const std::string& currentLabel() const;

private:
	// -------- Callbacks...
	void draw(const gfx::RenderContext& ctx) const override;

	void onActivationChanged(ActivationState state) override;
	void onMouseMoved(float x, float y) override;
	void onMousePressed(float x, float y) override;
	void onMouseReleased(float x, float y) override;
	void onMouseWheelMoved(int delta) override;

	void onKeyPressed(const sf::Event::KeyEvent& key) override;
	void onKeyReleased(const sf::Event::KeyEvent& key) override;
	void onThemeChanged() override;

	// -------- Helpers...
	// Change the currently selected item (without notification!)
	auto update_selection(size_t index);
	void update_arrow_pressed_state(ItemBox<Arrow>& arrow, float x, float y);

	// -------- Data...
	struct Item
	{
		std::string label;
		T value;

		Item(const std::string& text, const T& value);
	};

	typedef std::vector<Item> ItemVector;
	ItemVector m_items;
	size_t m_currentIndex;

	// Visual components
	ItemBox<Text> m_box;         // The entire widget (incl. the arrows)
	ItemBox<Arrow> m_arrowLeft;  // Control for "Select Prev."
	ItemBox<Arrow> m_arrowRight; // Control for "Select Next"
};

} // namespace

#include "OptionsBox.inl"

#endif // _SFW_OPTIONSBOX_HPP_
