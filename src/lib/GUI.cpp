#include "sfw/GUI-main.hpp"
#include "sfw/Theme.hpp"
#include "sfw/Widgets/Tooltip.hpp"
#include "sfw/util/diagnostics.hpp"

//!! Stuff for clearing the bg. when not owning the entire window
//!! (Should be moved to the Renderer!)
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Color.hpp>

#include <charconv>
#include <system_error>
#include <iostream> // for printing errors/warnings
using namespace std;

namespace sfw
{

// See the header for more on this:
GUI* GUI::DefaultInstance = nullptr;


//----------------------------------------------------------------------------
GUI::GUI(sf::RenderWindow& window, const sfw::Theme::Cfg& themeCfg, bool own_the_window):
	m_error(), // no error by default
	m_window(window),
	m_own_window(own_the_window), //!!Rename to sg. like manage_host_window
	m_themeCfg(themeCfg)
{
	// "Officially" mark this object as the "Main" in the GUI Widget tree:
	setParent(this);

	// Also register ourselves to our own widget registry:
	widgets["/"] = this;

	// Add as the "most recent" GUI manager:
	if (GUI::DefaultInstance) {
		cerr << "- Note: overriding previous default GUI instance (" << DefaultInstance << " with newly created " << this << ")\n";
	}
	GUI::DefaultInstance = this;

	reset();
}


//----------------------------------------------------------------------------
bool GUI::active()
{
	// Inactive, if:
	// - the gui has been (explicitly) closed
	// - it's in "error state"
	// - the (hosting) window is not open (regardless of owning it or not)
	return !m_closed && !m_error && m_window.isOpen();
}


//----------------------------------------------------------------------------
bool GUI::reset()
{
	m_error = std::error_code();

//!! This isn't useful here at all, as the generic layout resizer will
//!! shrink it back down dynamically anyway...:
//!!    if (m_own_window)
//!!        setSize(m_window.getSize());
//!! Instead, an overridden getSize() could use the dynamic size in case of
//!! owning the full window.

	setTheme(m_themeCfg); //! Should this be reset to some internal safe default instead?
						  //! And then should also even the widgets be deleted, perhaps? :-o
						  //! No. Better to destroy the entire GUI for that. And for a
						  //! blank slate a new GUI instance can always be created.
						  //! Which then means...: the current config should also NOT be zapped!

	//! Can't set a wallpaper just yet, as the size may be unknown yet!
	//! Widgets are yet to be added! See onResized()!

	m_cursorType = sf::Cursor::Type::Arrow;

	return (bool)*this;
}


//----------------------------------------------------------------------------
void GUI::close()
{
	m_closed = true;

	// If we control the window (not just the widgets), close that, too:
	if (m_own_window)
		m_window.close();
}

//----------------------------------------------------------------------------
bool GUI::closed()
{
	return m_closed;
}

//----------------------------------------------------------------------------
bool GUI::process(const sf::Event& event)
{
	thread_local bool event_processing_started = false;

	if (!event_processing_started)
	{
		//!!Call the subscription-based onEventProcessingStarted() notifications!...
		//!!This one is just hacked in here for now (to give it a chance to reapply
		//!!its placement according to the "final" GUI state):
		setWallpaper();

		event_processing_started = true;
	}

	if (!active()) return false;

	switch (event.type)
	{
	case sf::Event::MouseMoved:
	{
		sf::Vector2f mouse = convertMousePosition(event.mouseMove.x, event.mouseMove.y);
		onMouseMoved(mouse.x, mouse.y);
		break;
	}

	case sf::Event::MouseButtonPressed:
		if (event.mouseButton.button == sf::Mouse::Button::Left)
		{
			sf::Vector2f mouse = convertMousePosition(event.mouseButton.x, event.mouseButton.y);
			onMousePressed(mouse.x, mouse.y);
		}
		break;

	case sf::Event::MouseButtonReleased:
		if (event.mouseButton.button == sf::Mouse::Button::Left)
		{
			sf::Vector2f mouse = convertMousePosition(event.mouseButton.x, event.mouseButton.y);
			onMouseReleased(mouse.x, mouse.y);
		}
		break;

	case sf::Event::MouseWheelScrolled:
		onMouseWheelMoved((int)event.mouseWheelScroll.delta);
		break;

	case sf::Event::KeyPressed:
		onKeyPressed(event.key);
		break;

	case sf::Event::KeyReleased:
		onKeyReleased(event.key);
		break;

	case sf::Event::TextEntered:
		onTextEntered(event.text.unicode);
		break;

	case sf::Event::Closed:
	close();
		return false;
	break;

	default:
		break;
	}

	return true;
}


//----------------------------------------------------------------------------
bool GUI::setTheme(const sfw::Theme::Cfg& themeCfg)
{
//traverse([&](Widget* w) { cerr << w->getName() << "\n"; } );

	//!!Should be transactional, with no side-effects if failed!

	if (&m_themeCfg != &themeCfg)
	{
		m_themeCfg = themeCfg;
	}

	// Do this even if the config has not been changed, to allow calling from the ctor!
	if (!m_themeCfg.apply())
	{
		m_error = make_error_code(errc::invalid_argument); //!!Should actually be a custom one!
		cerr << "- ERROR: Failed to setup theme!\n";
		return false;
	}

	// OK, tell everyone about what just happened
	themeChanged();

	return true;
}

void GUI::themeChanged()
{
	// Notify widgets of the change...

	// Start with the GUI itself
	onThemeChanged();

	// Then all the rest
	traverse([](Widget* w) { w->onThemeChanged(); } );
	// Another reason to not use the widget registry map (besides "general
	// frugalism", and that it's not intended for this (i.e. it may not
	// even contain every widget!), is that std::map reorders its content
	// alphabetically, which is pretty awkward here (e.g. for diagnostics).
	// (Note: std::unordered_map won't preserve the original order either!)

	//! This would be redundant in the current model:
	//!traverse([](Widget* w) { w->recomputeGeometry(); } );
	//! The onThemeChanged() call typically involves setSize() too, which
	//! in turn also calls parent->recomputeGeometry() (via Widget::setSize)!
	//! (NOTE: there are ample chances of infinite looping here too, some of
	//! which I've duly explored already...)
}


//----------------------------------------------------------------------------
void GUI::renderBackground() //!!Move this to the renderer!
{
	if (sfw::Theme::clearBackground)
	{
		if (m_own_window)
		{
			m_window.clear(Theme::bgColor);
		}
		else
		{
			// Just clear the GUI rect
			sf::RectangleShape bgRect(getSize());
			bgRect.setPosition(getPosition());
			bgRect.setFillColor(Theme::bgColor);
			bgRect.setOutlineThickness(0);
			//!!renderer.draw(bgRect);...
			m_window.draw(bgRect);
		}
	}

	// The wallpaper is an overlay on top of the background color!
	// (It would become apparent e.g. when trying to change the transparency
	// of an existing one: it won't work without erasing the prev. first!)
	if (hasWallpaper())
	{
		m_window.draw(m_wallpaper); // Will (supposedly ;) ) only draw into the GUI rect!
	}
}

//----------------------------------------------------------------------------
void GUI::render()
{
	if (!active()) return;

	// Hitchhike the per-frame draw routine for updating the session time...
	onTick();

	/*m_renderer.*/renderBackground();

	// Draw whatever we have, via our a top-level widget container ancestor
	/*m_renderer.*/draw(gfx::RenderContext{m_window, sf::RenderStates()}); //! function-style RenderContext(...) failed with CLANG
}


void GUI::draw(const gfx::RenderContext& ctx) const
{
	VBox::draw(ctx);

	// Separate round for tooltips, to ensure they're on the top
	//!!Still, if multiple tooltips are too close to each other, and the most recent is iterated
	//!!earlier, it WILL NOT be the topmost! :-/ Proper stacking or explicit Z-ordering is required!
	//!!
	//!!Alas, widget-local, the nested drawing contexts available at the recursive
	//!!draw() calls do not exist here, with this flat widget iteration, so we have to
	//!!replicate that using that Transform() hack below... :-/ (See also: #315)
	//!!
	auto widget_ctx = ctx;
	ctraverse([&widget_ctx](const Widget* w) {
		if (w->m_tooltip) {
			auto widget_pos = w->getParent()->getAbsolutePosition();
			//!!SFML-specific:
			widget_ctx.props.transform = sf::Transform(
				1, 0, widget_pos.x,
				0, 1, widget_pos.y,
				0, 0, 1);

			w->m_tooltip->draw(widget_ctx);
		}
	});
}


//----------------------------------------------------------------------------
bool GUI::remember(Widget* widget, string name, bool override_existing)
{
	if (name.empty())
	{
		// Make the default name hex, for a more climactic debug experience...:
		char defname[17] = {0}; to_chars(defname, std::end(defname), (size_t)(void*)widget, 16);
		name = string(defname);
		// Paranoid sanity-checking of this default name
		if (widgets[name] != nullptr)
		{
			cerr << "- Warning: Widget ["<<name<<"] has already been registered with this default name.\n";
			assert(widgets[name] == widget);
			return true;
		}
	}
	else if (auto other_it = widgets.find(name); other_it != widgets.end()) // Someone has this name aleady?
	{
		Widget* other = other_it->second;
		if (other == widget)
		{
			cerr << "- Warning: Repeated registration of widget as \"" << name << "\".\n";
			return true;
		}

		if (!override_existing)
		{
			cerr << "- Warning: Refusing to override the name (\"" << name << "\") of another widget.\n";
			return false;
		}
		else
		{
			cerr << "- Warning: Another widget has already been registered as \"" << name << "\".\n"
			     << "  Overriding...\n";

			// Forget the name of the other widget:
			[[maybe_unused]] auto result_of_forgetting = remember(other, "");
			assert(result_of_forgetting);

			// Fall through to assign `name` to `widget`...
		}
	}
	
	widgets[name] = widget;
	return true;
}

//----------------------------------------------------------------------------
Widget* GUI::recall(std::string_view name) const
{
	string name__cpp_double_bullshit{name}; // a) Can't just map.find(string_viuw), b) can't just string = string_view

	auto widget_iter = widgets.find(name__cpp_double_bullshit);
	if (widget_iter == widgets.end()) cerr << "- Warning: widget \"" << name__cpp_double_bullshit << "\" not found!\n";
//if (widget_iter != widgets.end()) cerr << "recall: found widget: \"" << widget_iter->first << "\" == "<< (void*)widget_iter->second <<"\n";
	return widget_iter != widgets.end() ? widget_iter->second : (Widget*)nullptr;
}

string GUI::recall(const Widget* w) const
{
	for (const auto& [name, widget]: widgets)
	{
		if (widget == w) return name;
	}
	return "";
}


//----------------------------------------------------------------------------
sf::Vector2f GUI::getMousePosition() const
{
	auto pixpos = sf::Mouse::getPosition(m_window);
	return convertMousePosition(pixpos.x, pixpos.y);
}

//----------------------------------------------------------------------------
sf::Vector2f GUI::convertMousePosition(int x, int y) const
{
	sf::Vector2f mouse = m_window.mapPixelToCoords(sf::Vector2i(x, y));
	mouse -= getPosition();
	return mouse;
}

//----------------------------------------------------------------------------
void GUI::setMouseCursor(sf::Cursor::Type cursorType)
{
	if (cursorType != m_cursorType)
	{
		if (Theme::cursor.loadFromSystem(cursorType))
		{
			m_window.setMouseCursor(Theme::cursor);
			m_cursorType = cursorType;
		}
	}
}


//----------------------------------------------------------------------------
void GUI::setWallpaper(const Wallpaper::Cfg& cfg)
{
	if (cfg.filename.empty())
		m_wallpaper.disable(); // There may be one from a previous theme -- ditch it!
	else
		setWallpaper(cfg.filename, cfg.placement, cfg.tint);
}

void GUI::setWallpaper(std::string filename, Wallpaper::Placement placement, sf::Color tint)
{
	// This may also get called way too early, even before any widgets
	// are added (i.e. from the initial setTheme() via the ctor!), so
	// better not waste time/resources then:
	if (!getSize().x || !getSize().y)
		return;

	if (filename.empty())
	{
		filename = Theme::wallpaper.filename;
		placement = Theme::wallpaper.placement;
		tint = Theme::wallpaper.tint;
	}
	if (filename.empty())
		return;

	m_wallpaper.setImage(filename);
	m_wallpaper.setSize(getSize()); //!!Rename it to `setWallSize` or sg. more expressive!
	m_wallpaper.setColor(tint);
	m_wallpaper.enable();

	//!! This shoud be done by the wallpaper itself!
	//!!using Wallpaper::Placement;
	switch (placement)
	{
	case Wallpaper::Center: //!!See above for avoiding Wallpaper::
		break;
	default:;
	}
}

void GUI::setWallpaperColor(sf::Color tint)
{
	m_wallpaper.setColor(tint);
}

bool GUI::hasWallpaper()
//!! Define the exact meaning of this query!
//!! Checking only for m_wallpaper can only tell the current state of the current wallpaper,
//!! which may be disabled/missing etc. for whatever reason, but clients might ask this question
//!! to find out about the _intent_! (In which case the theme config must also be checked.)
{
	return m_wallpaper; //!!?? && !Theme::wallpaper.filename.empty()
}

void GUI::disableWallpaper()
{
	m_wallpaper.disable();
	assert(!m_wallpaper);
}


void GUI::setPosition(const sf::Vector2f& pos)
{
	if (hasWallpaper()) m_wallpaper.setPosition(pos);
	Widget::setPosition(pos);
}

void GUI::setPosition(float x, float y)
{
	setPosition(sf::Vector2f(x, y));
}


sf::Vector2f GUI::getSize() const
{
	return m_own_window ? sf::Vector2f{(float)m_window.getSize().x, (float)m_window.getSize().y}
	                    : Widget::getSize();
}

float GUI::sessionTime() const
{
	return m_sessionTime.asSeconds();
}


//----------------------------------------------------------------------------
// Callbacks...
//----------------------------------------------------------------------------

void GUI::onThemeChanged()
{
	setWallpaper(Theme::wallpaper);
}


void GUI::onResized()
{
#ifdef DEBUG
//cerr <<"resized to " <<getSize().x <<" x "<<getSize().y <<" /" <<sessionTime() <<endl;
#endif
	m_wallpaper.setSize(getSize());
}


void GUI::onTick()
{
	m_sessionTime += m_clock.restart();

	//!! Go through the set of registered timer callbacks to check if
	//!! any of them are (over)due, and call those. (Note: most of them
	//!! may have requested triggering on (relative) timeouts, rather than
	//!! on "absolute" session time!)
	//!! ...
	//!! Then remove the one-shot hooks:
	//!! ...
	//!!-----------------------------------------
	//!! Or, alternatively(?):
	//!! Just call every widget's onTick()...
	//!! Compilers should optimize out the empty default onTick() virtuals... right?... RIGHT???
	traverse([](Widget* w) {
		w->onTick();
		//!!Manual kludge until Widget becomes WidgetContainer, so tooltips can be proper tree nodes:
		if (w->m_tooltip && w->m_tooltip->armed()) w->m_tooltip->onTick();
	});
}

} // namespace
