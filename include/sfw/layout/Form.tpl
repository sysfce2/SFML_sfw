	//
	// add("label", new MyRealWidget) -> MyRealWidget*
	//
	template <class W> requires (std::is_base_of_v<Widget, W>)
	W* add(std::string_view label, W* widget, std::string_view widgetname_override = "")
		{ return (W*)(Widget*)add(label, (Widget*)widget, widgetname_override); }

	//
	// add("label", MyRealWidget()) -> MyRealWidget*
	//
	template <class W> requires (std::is_base_of_v<Widget, W>)
	W* add(std::string_view label, W&& tmpWidget, std::string_view name = "")
		{ return (W*)(Widget*)add(label, (Widget*)new W(std::move(tmpWidget)), name); }


	/*************************************************************************
	* Same as above, but allowing any widget as "label"
	*************************************************************************/

	//
	// add(new MyComplexLabel, new RealWidget) -> MyRealWidget*
	//
	template <class W> requires (std::is_base_of_v<Widget, W>)
	W* add(Widget* labelwidget, W* widget,
	       std::string_view widgetname_override = "", std::string_view labelwidgetname_override = "")
		{ return (W*)(Widget*)add((Widget*)labelwidget, (Widget*)widget,
					widgetname_override, labelwidgetname_override); }

	//
	// add(new MyComplexLabel, RealWidget()) -> MyRealWidget*
	//
	template <class W> requires (std::is_base_of_v<Widget, W>)
	W* add(Widget* labelwidget, W&& tmpWidget,
	       std::string_view widgetname_override = "", std::string_view labelwidgetname_override = "")
		{ return (W*)(Widget*)add((Widget*)labelwidget, (Widget*)new W(std::move(tmpWidget)),
					widgetname_override, labelwidgetname_override); }


	/*************************************************************************
	* Same as above, but also allowing add(Label(), new W) and add(Label(), W())
	*************************************************************************/

	//
	// add(MyComplexLabel(), new RealWidget) -> MyRealWidget*
	//
	template <class W, class L> requires (std::is_base_of_v<Widget, W> && std::is_base_of_v<Widget, L>)
	W* add(L&& tmpLabelWidget, W* widget,
	       std::string_view widgetname_override = "", std::string_view labelwidgetname_override = "")
		{ return (W*)(Widget*)add((Widget*)new L(std::move(tmpLabelWidget)), (Widget*)widget,
					widgetname_override, labelwidgetname_override); }

	//
	// add(MyComplexLabel(), RealWidget()) -> MyRealWidget*
	//
	template <class W, class L> requires (std::is_base_of_v<Widget, W> && std::is_base_of_v<Widget, L>)
	W* add(L&& tmpLabelWidget, W&& tmpWidget,
	       std::string_view widgetname_override = "", std::string_view labelwidgetname_override = "")
		{ return (W*)(Widget*)add((Widget*)new L(std::move(tmpLabelWidget)), (Widget*)new W(std::move(tmpWidget)),
					widgetname_override, labelwidgetname_override); }
