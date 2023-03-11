#include "sfw/GUI.hpp"

#include <SFML/Graphics.hpp>

#include <string> // to_string
#include <iostream> // cerr, for errors, cout for some "demo" info
#include <cassert>
using namespace std;

int main()
{
	//------------------------------------------------------------------------
	// SFML app frame....
	sf::RenderWindow window;
	window.create(sf::VideoMode({1024, 768}), "SFW TEST: main", sf::Style::Close|sf::Style::Resize);
	if (!window.isOpen()) {
		cerr << "- Failed to create the SFML window!\n";
		return EXIT_FAILURE;
	}
	window.setFramerateLimit(30);

	//------------------------------------------------------------------------
	// Test GUI setuo...

	using namespace sfw;
	using OBColor = sfw::OptionsBox<sf::Color>;

	Theme::DEFAULT.basePath = "demo/";
	Theme::DEFAULT.fontFile = "font/Vera.ttf";

	Theme::PADDING = 2.f;
	Theme::click.textColor      = hex2color("#191B18");
	Theme::click.textColorHover = hex2color("#191B18");
	Theme::click.textColorFocus = hex2color("#000");
	Theme::input.textColor = hex2color("#000");
	Theme::input.textColorHover = hex2color("#000");
	Theme::input.textColorFocus = hex2color("#000");
	Theme::input.textSelectionColor = hex2color("#8791AD");
	Theme::input.textPlaceholderColor = hex2color("#8791AD");

	// Some dynamically switcahble theme "quick config packs" to play with
	Theme::Cfg themes[] = {
		{ "Default (\"Baseline\")", "demo/", "texture-sfw-baseline.png", hex2color("#e6e8e0"), 11, "font/Liberation/LiberationSans-Regular.ttf" },
		{ "Classic ☺",              "demo/", "texture-sfw-classic.png",  hex2color("#e6e8e0"), 12, "font/Liberation/LiberationSans-Regular.ttf" },
		{ "sfml-widgets's default", "demo/", "texture-sfmlwidgets-default.png", hex2color("#dddbde"), },
		{ "sfml-widgets's Win98",   "demo/", "texture-sfmlwidgets-win98.png",   hex2color("#d4d0c8"), },
	};
	const size_t DEFAULT_THEME = 0;


	//--------------------------------------------------------------------
	// Creating the main GUI controller:
	sfw::GUI demo(window, themes[DEFAULT_THEME]);
	if (!demo) {
		return EXIT_FAILURE; // Errors have already been printed to cerr.
	}
	demo.setPosition(10, 10);

	demo.add((new Label("––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––"
	                   " MOST RECENT TEST CASES: "
	                   "––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––")));//->setStyle(sf::Text::Style::Bold));
	//--------------------------------------------------------------------
	// Part of #167: Add a generic "SFML container" widget, and put the demo text into one
	// (See the rest of #167 later below, at the other DrawHost widget!)
	auto circlevista = new DrawHost([&](auto* w, auto ctx) {
		sf::CircleShape circ(50);
		sf::Texture backdrop;
                if (!backdrop.loadFromFile("test/thumbnail.jpg")) return;
		circ.setTexture(&backdrop);
		circ.setTextureRect({{10, 10}, {100, 100}});

		auto sfml_renderstates = ctx.props;
		sfml_renderstates.transform *= w->getTransform();
		ctx.target.draw(circ, sfml_renderstates);
#ifdef DEBUG
		w->draw_outline(ctx);
#endif
	});
	circlevista->setSize(100,100);

	// #168: Form supporting any left-hand-side widget as "label"
	auto labelbox = new VBox;
		labelbox->add(Label("Issue #168"));
		labelbox->add(circlevista);

	auto test_hbox = demo.add(new HBox);
	test_hbox->add(new Form)->add(labelbox, new Label("OK!"));
	//!!#171, more of #168 (now with Image*):
	//!!test_hbox->add(new Form)->add((new Image("test/thumbnail.jpg"))->rescale(0.5), Label("#171 OK, too!"));
	//!!#171, template-move also for the "label":
	//!!test_hbox->add(new Form)->add(Image("test/thumbnail.jpg").rescale(0.5), Label("#171 OK, too!"));

	//!! This is not yet supported (nor separators...):
	//!!test_hbox->add(new Form)->add("This is just some text on its own.");

	auto issue127box = test_hbox->add(new VBox);
	// #127 + name lookup
	issue127box->add(sfw::Button("Issue #127/void", [&] {
		((sfw::Button*)(demo.get("test #127")))->setText("Found itself!");
	}), "test #127");
	issue127box->add(sfw::Button("Issue #127/w", [&](auto* w) { cerr << w << ", " << demo.get("test #127/w") << endl;
		((sfw::Button*)(w->get("test #127/w")))->setText("Found itself!");
	}), "test #127/w");


	demo.add((new Label("––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––"
	                   "(proper separators are not yet supported...)"
	                   "––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––"))->setStyle(sf::Text::Style::Italic));

	//--------------------------------------------------------------------
	// Some raw SFML shapes (sf::Text + a rect) to be manipulated via the GUI
	// + a DrawHost for boxing them nicely, as per #167

        //!! This needs to go into a w->setup() function, and actually that's where
	//!! the draw hook could also be set, optionally...
	//!! Which kinda implies two ctor forms for the two different hooks
	//!! (and fortuantely, the hook signatures differ! :) ), and of course
	//!! two setters for hooking Setup and Draw.
	//!!
	//!! Plus, a light (template-based?) interface would also be needed
	//!! for accessging them from the outside in a somewhat civilized way!
		sf::Text text("Hello world!", Theme::getFont());
		sf::RectangleShape textrect;

	auto sfText = new DrawHost([&](auto* raw_w, auto ctx) {
		// Get current (raw, untransformed) text size
		//! Note: the text string & style attributes may have been
		//! changed by other widgets!
		auto tbounds = text.getLocalBounds();
		//! sf::Text's bound-rect is not 0-based! :-o (-> SFML #216)
		sf::Vector2f mysterious_sfml_offset = {tbounds.left, tbounds.top};
		sf::Vector2f tsize = {tbounds.width + tbounds.left, tbounds.height + tbounds.top};
		// Get boundrect of the transformed (rotated, scaled etc.) text
		// for sizing the wrapper widget
		sf::FloatRect bgRect{{0,0}, {tsize.x * 1.2f, tsize.y * 3.5f}};
		auto actualBoundRect = text.getTransform().transformRect(bgRect);
//!!??		auto actualBoundRect = xform.transformRect(textrect.getGlobalBounds());
//!!?? Why's getGlobalBounds different here?!
//!!?? It does make the widget resize correctly -- which is the only place this
//!!?? rect is used --, but then the shape itself will stay the same size, and the
//!!?? rotation would be totally off! :-o It's all just queries! WTF?! :-ooo

		auto* w = (DrawHost*)raw_w;

		//!! Umm... This is *GROSS*! Basically triggers a full GUI resize, right from draw()! :-oo
		w->setSize((actualBoundRect.width  + 2 * Theme::PADDING),
		           (actualBoundRect.height + 2 * Theme::PADDING));

		//!! Must position the shapes after setSize, as setSize may move the widget
		//!! out from the content we manually hack here, leaving the appearance off
		//!! for the current frame! :-o (A very visible artifact, as I've learned!...)
		//!! Well, another disadvantage of calling setSize during draw()!...
		//!! Fortunately, this one can be tackled by doing this after setSize:

		// Sync the bg. rect to various "externalia":
		textrect.setScale(text.getScale());
		textrect.setRotation(text.getRotation());
		textrect.setSize(bgRect.getSize());
		textrect.setOrigin({textrect.getSize().x / 2, textrect.getSize().y / 2});
		textrect.setPosition({w->getSize().x / 2, w->getSize().y / 2});

		text.setOrigin({tsize.x / 2 + mysterious_sfml_offset.x,
                                tsize.y / 2 + mysterious_sfml_offset.y}); //! sf::Text's bound-rect is not 0-based! :-o
		text.setPosition({w->getSize().x / 2, w->getSize().y / 2});

		auto sfml_renderstates = ctx.props;
		sfml_renderstates.transform *= w->getTransform();
		ctx.target.draw(textrect, sfml_renderstates);
		ctx.target.draw(text, sfml_renderstates);
#ifdef DEBUG
		w->draw_outline(ctx);
#endif
	});


	//--------------------------------------------------------------------
	// A horizontal layout for multiple panels side-by-side
	auto main_hbox = demo.add(HBox());


	//--------------------------------------------------------------------
	// A "form" panel on the left
	auto form = main_hbox->add(Form());

	// A text box to set the text of the example SFML object (created above)
	/*!! #171: this won't compile yet:
	form->add("Text", new sfw::TextBox())
		->setPlaceholder("Type something!")
		->setText("Hello world!")
		->setCallback([&](auto* w) { text.setString(w->getText()); });
	*/
	// Another text box, with length limit & pulsating cursor
	// Creating it without a "junk" local variable now:
	form->add("Text with limit (5)", (new TextBox(50.f, TextBox::CursorStyle::PULSE))
		->setMaxLength(5)
		->setText("Hello world!")
	);
	// The label above ("Text with limit (5)") will be used for retrieving
	// the widget later by the crop slider (somewhere below)!

	// Slider + progress bars for rotating
	auto sliderForRotation = new sfw::Slider(1); // granularity = 1%
	auto pbarRotation1 = new ProgressBar(200.f, sfw::Horizontal, sfw::LabelNone);
	auto pbarRotation2 = new ProgressBar(200.f, sfw::Horizontal, sfw::LabelOver);
	auto pbarRotation3 = new ProgressBar(200.f, sfw::Horizontal, sfw::LabelOutside);
	sliderForRotation->setCallback([&](auto* w) {
		text.setRotation(sf::degrees(w->getValue() * 360 / 100.f));
		pbarRotation1->setValue(w->getValue());
		pbarRotation2->setValue(w->getValue());
		pbarRotation3->setValue(w->getValue());
	});
	// Slider + progress bars for scaling
	auto sliderForScale = new sfw::Slider(); // default granularity: 10%
	auto pbarScale1 = new ProgressBar(100, sfw::Vertical, sfw::LabelNone);
	auto pbarScale2 = new ProgressBar(100, sfw::Vertical, sfw::LabelOver);
	auto pbarScale3 = new ProgressBar(100, sfw::Vertical, sfw::LabelOutside);
	sliderForScale->setCallback([&](auto* w) {
		float scale = 1 + w->getValue() * 2 / 100.f;
		text.setScale({scale, scale});
		pbarScale1->setValue(w->getValue());
		pbarScale2->setValue(w->getValue());
		pbarScale3->setValue(w->getValue());
	});

	form->add("Rotation", sliderForRotation);
	form->add("Scale", sliderForScale);

	// Color selectors
	// -- Creating one as a template to clone it later, because
	//    WIDGETS MUSTN'T BE COPIED AFTER HAVING BEEN ADDED TO THE GUI!
	OBColor ColorSelect_TEMPLATE; (&ColorSelect_TEMPLATE)
		->addItem("Black", sf::Color::Black)
		->addItem("Red", sf::Color::Red)
		->addItem("Green", sf::Color::Green)
		->addItem("Blue", sf::Color::Blue)
		->addItem("Cyan", sf::Color::Cyan)
		->addItem("Yellow", sf::Color::Yellow)
		->addItem("White", sf::Color::White)
	;

	// Text color selector
	auto optTxtColor = (new OBColor(ColorSelect_TEMPLATE))
		->setCallback([&](auto* w) {
			text.setFillColor(w->getSelectedValue());
			w->setColor(w->getSelectedValue());
		});

	// Text backgorund rect color
	auto optTxtBg = (new OBColor(ColorSelect_TEMPLATE))
		->addItem("Default", themes[DEFAULT_THEME].bgColor)
		->setCallback([&](auto* w) {
			textrect.setFillColor(w->getSelectedValue());
			w->setColor(w->getSelectedValue());
		});

	// See also another one for window background color selection!

	form->add("Text color", optTxtColor);
	form->add("Text bg.", optTxtBg);

	// Checkboxes to set text properties
	// ..."classic" `add(new Widget(...))` style, setting properties in the ctor.
	// NOTE: It's generally unsafe to set the properties of a free-standing
	//       (newly created, but not-yet-added) widget!
	//       Only those supported by the constructor are guaranteed to work.
	form->add("Underlined text", new sfw::CheckBox([&](auto* w) {
		int style = text.getStyle();
		if (w->isChecked()) style |= sf::Text::Underlined;
		else                style &= ~sf::Text::Underlined;
		text.setStyle(style);
	}));
	// ..."templated" add(Widget())->set...() style, setting properties
	// on the "real" (added) widget:
	form->add("Bold text", sfw::CheckBox())->setCallback([&](auto* w) {
		int style = text.getStyle();
		if (w->isChecked()) style |= sf::Text::Bold;
		else                style &= ~sf::Text::Bold;
		text.setStyle(style);
	});

	// Attach the horiz. progress bars (used for text rotation) to the form
	form->add("Progress bar (label = None)",    pbarRotation1);
	form->add("Progress bar (label = Over)",    pbarRotation2);
	form->add("Progress bar (label = Outside)", pbarRotation3);

	// Attach the vert. progress bars (used for text scaling) to a new box
	auto layoutForVerticalProgressBars = new sfw::HBox();
	//!! Issue #109: Document that manipulating unowned, free-standing layouts is UB!
	form->add("Vertical progress bars", layoutForVerticalProgressBars);
	layoutForVerticalProgressBars->add(pbarScale1);
	layoutForVerticalProgressBars->add(pbarScale2);
	layoutForVerticalProgressBars->add(pbarScale3);

	form->add("Default button", sfw::Button("button"));
	cout << "Button text retrieved via name lookup: \"" << ((sfw::Button*)demo.get("Default button"))->getText() << "\"\n";

	form->add("UTF-8 test", sfw::Button("VÍZ- GÁZ- ÉS HÓTŰRŐ")); // Note: this source is already UTF-8 encoded.
	cout << "UTF-8 button text got back as: \"" << ((sfw::Button*)demo.get("UTF-8 test"))->getText() << "\"\n";

	// Custom bitmap button
	sf::Texture buttonimg; //! DON'T put this inside the if () as local temporary (as I once have... ;) )
	if (buttonimg.loadFromFile("demo/sfmlwidgets-themed-button.png")) // SFML would print an error if failed
	{
		form->add("Custom button", (new sfw::ImageButton(buttonimg, "Play"))->setTextSize(20)
			->setCallback([]/*(auto* w)*/ { /*no-arg. compilation test*/ }));
	}


	//--------------------------------------------------------------------
	// A panel in the middle
	auto middle_panel = main_hbox->add(sfw::VBox());

	// OK, add the SFML test shapes + adapter widget here
	middle_panel->add(sfText);

	// Textbox for the labels of new buttons created by the button-factory button...
	auto boxfactory = middle_panel->add(sfw::HBox());
	auto labeller = boxfactory->add(sfw::TextBox(100))->setText("Edit Me!")->setPlaceholder("Button label");
	boxfactory->add(sfw::Button("Create button", [&] {
		middle_panel->addAfter(boxfactory, new sfw::Button(labeller->getText()));
	}));


	//--------------------------------------------------------------------
	// Image views...

	// Image directly from file
	auto vboximg = main_hbox->add(sfw::VBox());
	vboximg->add(sfw::Label("Image from file:"));
	vboximg->add(sfw::Image("demo/some image.png"));

	// Image crop from file
	vboximg->add(sfw::Label("Crop from file:"));
	vboximg->add(sfw::Image("demo/some image.png", {{0, 33}, {24, 28}}));
	vboximg->add(sfw::Label("Image crop varied:"));

	// Image from file, cropped dynamically
	sfw::Image* imgCrop = new sfw::Image("demo/martinet-dragonfly.jpg");

	// Slider & progress bar for cropping an Image widget
	vboximg->add(sfw::Slider(1, 100))->setCallback([&](auto* w) {
		((sfw::ProgressBar*)w->get("cropbar"))->setValue(w->getValue());
		// Show the slider value in a text box retrieved by its name:
		auto tbox = (sfw::TextBox*)w->get("Text with limit (5)");
		if (!tbox) cerr << "Named TextBox not found! :-o\n";
		else tbox->setText(to_string((int)w->getValue()));
		imgCrop->setCropRect({{(int)(w->getValue() / 4), (int)(w->getValue() / 10)},
		                      {(int)(w->getValue() * 1.4), (int)w->getValue()}});
	});

	auto boxcrop = vboximg->add(new sfw::HBox);
		boxcrop->add(sfw::Label("Crop square size:"));
		boxcrop->add(sfw::ProgressBar(40), "cropbar");

	vboximg->add(imgCrop);
	vboximg->add(sfw::Label("(Art: © Édouard Martinet)"))->setStyle(sf::Text::Style::Italic);


	//--------------------------------------------------------------------
	// Another "sidebar" column, for (theme) introspection...
	auto right_bar = main_hbox->add(sfw::VBox());

	// Theme selection
	using OBTheme = sfw::OptionsBox<Theme::Cfg>;
	right_bar->add(sfw::Label("Change theme:"));
	auto themeselect = new OBTheme([&](auto* w) {
		const auto& themecfg = w->getSelectedValue();
		demo.setTheme(themecfg); // Swallowing the error for YOLO reasons ;)
	});
	for (auto& t: themes) { themeselect->addItem(t.name, t); }
	themeselect->selectItem(DEFAULT_THEME);
	right_bar->add(themeselect, "theme-selector");

	// Theme font size slider
	// (Changes the font size directly of the cfg. data stored in "theme-selector",
	// so it will remember the new size(s)!)
	right_bar->add(sfw::Label("Theme font size (use the m. wheel):"));
	right_bar->add(sfw::Slider(10, 100))
		->setValue(30)
		->setCallback([&] (auto* w){
			assert(w->get("theme-selector"));
			auto& themecfg = ((OBTheme*)(w->get("theme-selector")))->getSelectedValueRef();
			themecfg.textSize = 8 + size_t(w->getValue() / 10);
			demo.setTheme(themecfg);
		});

	// Show the current theme texture bitmaps
	right_bar->add(sfw::Label("Theme textures:"));
	auto txbox = right_bar->add(new HBox);
	struct ThemeBitmap : public Image {
		ThemeBitmap(float zoom) : Image(Theme::getTexture()) { scale(zoom); }
		void onThemeChanged() override {
			setTexture(Theme::getTexture()); // note: e.g. the ARROW is at {{0, 42}, {6, 6}}
			scale(scale()); //!!Should do this itself!
		}
	};
	auto themeBitmap = new ThemeBitmap(2); // start with 2x zoom
	txbox->add(sfw::Slider(1.f, 100.f, sfw::Vertical))
		->setCallback([&](auto* w) { themeBitmap->scale(1 + (100.f - w->getValue()) / 25.f); })
		->setStep(25.f)
		->setValue(75.f);
	txbox->add(themeBitmap);

	right_bar->add(Label(" ")); // Just for some space, indeed...

	// Window background color selector
	right_bar->add(new Form)->add("Window bg.", (new OBColor(ColorSelect_TEMPLATE))
		->addItem("Default", themes[DEFAULT_THEME].bgColor)
		->setCallback([&](auto* w) {
			Theme::windowBgColor = w->getSelectedValue();
//			w->setColor(w->getSelectedValue());
		})
	);

	// A pretty useless, but interesting clear-background checkbox
	auto hbox4 = right_bar->add(new sfw::HBox);
	hbox4->add(sfw::Label("Clear background"));
	hbox4->add(sfw::CheckBox([&](auto* w) { Theme::clearWindow = ((sfw::CheckBox*)w->get("x"))->isChecked(); }, true), "x");


	// Custom exit button (also useless, but feels so nice! :) )
	demo.add(sfw::Button("Quit", [&] { window.close(); }));


	//--------------------------------------------------------------------
	// OK, GUI Setup done. Set some "high-level" defaults
	// (after setup, as these may trigger callbacks)
	//
	sliderForRotation->setValue(27);
	sliderForScale->setValue(20);
	//!!#160, too:
	optTxtColor->selectItem(2); // 3rd item; selectItem fails to trigger the callback!
	optTxtColor->selectPrevious(); // this one doesn't... ;)
		//!! Wow, but then it still fails to update the color, if triggered this way! :-o
		//!! Even though the code is there, and is apparently fine!... :-o WTF?!
		//!! (That's also part of #160 now...)

	optTxtBg->selectItem(1); optTxtBg->selectPrevious();

	//--------------------------------------------------------------------
	// The event loop
	while (window.isOpen())
	{
		demo.render();
		window.display();

		sf::Event event;
		while (window.pollEvent(event))
		{
			demo.process(event);

			if (event.type == sf::Event::Closed ||
			   (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
				window.close();
		}

	}

	return EXIT_SUCCESS;
}
