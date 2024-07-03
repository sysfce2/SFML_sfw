#ifndef _DIOFCUHIWOUETY8XYN867D49T86WDR7G4FH_
#define _DIOFCUHIWOUETY8XYN867D49T86WDR7G4FH_


#include "sfw/Layout.hpp"

#include <string_view>


namespace sfw
{

/**
 * Horizontally stacked layout with a label before each widget
 */
class Form: public Layout
{
public:
	Form();

	/*
	 * Add new row with a widget, prefixed by a label
	 * - label: Label for the widget. (Will also be used as its internal name.)
	 * - widget: Pointer to the widget to add (or none)
	 *	     If no widget, `label` is used as a separator line.
	 * - widgetname_override: Use this as the widget name (instead of `label`)
	 * Returns a pointer to the added widget (or nullptr for label-only rows).
	 */
	Widget* add(std::string_view label, Widget* widget = nullptr,
		std::string_view widgetname_override = "");

	/*
	 * As above, but allow using any widget as "label" (prefix)
	 * Note: can't assign default name to `widget` this way;
	 * use `widgetname_override`.
	 * - labelwidgetname_override: name for the "label" widget
	 */
	Widget* add(Widget* labelwidget, Widget* widget = nullptr,
		std::string_view widgetname_override = "", std::string_view labelwidgetname_override = "");

	/*************************************************************************
	* Various convenience helper templates are also defined to spare all the
	* upcasting from Widget*, so this would work as expected:
	*
	*     add("label", new TextBox)->setText("OK")
	*
	* and to also allow "new-less" forms like:
	*
	*     add("label", W())
	*     add(LabelWidget(), W())
	*************************************************************************/
	#include "Form.tpl"

private:
	void recomputeGeometry() override;

	float m_labelColumnWidth;

}; // class Form

} // namespace sfw


#endif // _DIOFCUHIWOUETY8XYN867D49T86WDR7G4FH_
