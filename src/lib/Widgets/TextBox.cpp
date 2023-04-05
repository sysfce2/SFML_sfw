#include "sfw/Widgets/TextBox.hpp"

#include "sfw/Theme.hpp"
#include "sfw/GUI-main.hpp"
#include <sfw/util/diagnostics.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/OpenGL.hpp>

#include <cassert>
#include <algorithm>
    using std::min, std::max;

namespace sfw
{

//----------------------------------------------------------------------------
// TextBox
//----------------------------------------------------------------------------

TextBox::TextBox(float width, CursorStyle style):
    m_maxLength(256),
    m_width(width),
    m_box(Box::Input)
{
    // Internal mechanics
    setCursorPos(0);

    // Visuals
    m_cursorStyle = style;
    onThemeChanged();
}


TextBox* TextBox::setText(const sf::String& string)
{
    m_text.setString(string.substring(0, m_maxLength)); // Limit the length
    setCursorPos(getText().getSize());

    return this;
}


const sf::String& TextBox::getText() const
{
    return m_text.getString();
}


TextBox* TextBox::setMaxLength(size_t maxLength)
{
    m_maxLength = maxLength;
    // Trim current text if needed
    if (getTextLength() > m_maxLength)
    {
        m_text.setString(m_text.getString().substring(0, m_maxLength));
        setCursorPos(m_maxLength);
    }

    return this;
}


void TextBox::setCursorPos(size_t index)
{

    if (index > getTextLength()) // NOTE: a) The cursor pos. is unsigned.
                                 //       b) The pos. right after the end (at EOS) is OK.
    {
        // Since a previously stopped (volatile) selection would need to be cancelled on
        // the next action -- even if it won't do anything (like move the cursor) --,
        // here we need to make sure a "null" navigation event also reaches the selection:
        m_selection.follow(getCursorPos());
        return;
    }

    m_cursorPos = index;
    m_selection.follow(index); // The selection itself will decide if and how exactly...

    // Adjust the visuals...

    float padding = Theme::borderSize + Theme::PADDING;
    m_cursorRect.setPosition({m_text.findCharacterPos(index).x, padding});
    m_cursorTimer.restart();

    auto pixel_cur_pos =  m_cursorRect.getPosition().x;
    if (pixel_cur_pos > getSize().x - padding)
    {
        // Shift left
        float diff = pixel_cur_pos - getSize().x + padding;
        m_text.move({-diff, 0});
        m_cursorRect.move({-diff, 0});
    }
    else if (pixel_cur_pos < padding)
    {
        // Shift right
        float diff = padding - pixel_cur_pos;
        m_text.move({diff, 0});
        m_cursorRect.move({diff, 0});
    }

    float textWidth = m_text.getLocalBounds().width;
    if (m_text.getPosition().x < padding && m_text.getPosition().x + textWidth < getSize().x - padding)
    {
        float diff = (getSize().x - padding) - (m_text.getPosition().x + textWidth);
        m_text.move({diff, 0});
        m_cursorRect.move({diff, 0});
        // If the text is smaller than the box, align left
        if (textWidth < (getSize().x - padding * 2))
        {
            diff = padding - m_text.getPosition().x;
            m_text.move({diff, 0});
            m_cursorRect.move({diff, 0});
        }
    }
}


//--- callbacks --------------------------------------------------------------

void TextBox::onKeyReleased(const sf::Event::KeyEvent& key)
{
    if (key.code == sf::Keyboard::LShift || key.code == sf::Keyboard::RShift)
    {
        m_selection.stop();
    }
}


void TextBox::onKeyPressed(const sf::Event::KeyEvent& key)
{
    switch (key.code)
    {
    case sf::Keyboard::LShift:
    case sf::Keyboard::RShift:
        // If there's a selection, that means it's just been stopped.
        // Pressing Shift again in that case very likely means an intent to
        // resume it (probably for adjusting its size etc.), so allow that.
        // (It'd feel like a hybrid volatile x pers. sel.)
        //
        // But... What if it's a mistaken selection and the user just wants
        // to start right over?! So, perhaps no...
        //if (m_selection) m_selection.resume();
        //else m_selection.start(m_cursorPos);
        m_selection.start(m_cursorPos);
        break;

    case sf::Keyboard::Left:
        if (key.control)
        {
            // NOTE: assuming getString()[size] is a valid ref. (of the trailing \0)!

            setCursorPos(m_cursorPos - 1);
            auto what_to_skip = m_text.getString()[m_cursorPos]; //! == ' ' will be checked to decide
            while (m_cursorPos > 0 &&
                   ((what_to_skip == ' ' && m_text.getString()[m_cursorPos - 1] == ' ')
                 || (what_to_skip != ' ' && m_text.getString()[m_cursorPos - 1] != ' '))) // sorry, the extra () noise is to shut GCC up (-Wparentheses)
                setCursorPos(m_cursorPos - 1);
        }
        else if (!key.shift && m_selection && getCursorPos() == m_selection.upper())
        {
            // This is a special case for better ergonomics:
            // Resume the disappearing selection, and move the cursor to the left of it,
            // because what's actually desired is almost never to finish a selection,
            // do nothing with it, and then move the cursor a bit just to kill it...
            m_selection.resume();
            m_selection.set_from_to(getCursorPos(), m_selection.lower());
            setCursorPos(m_selection.lower());
            m_selection.stop();
        }
        else
        {
            // Just move the cursor normally
            setCursorPos(m_cursorPos - 1);
        }
        break;

    case sf::Keyboard::Right:
        if (key.control)
        {
            // NOTE: assuming getString()[size] is a valid ref. (of the trailing \0)!

            auto what_to_skip = m_text.getString()[m_cursorPos]; //! == ' ' will be checked to decide
            do {
                setCursorPos(m_cursorPos + 1);
            } while (m_cursorPos < getTextLength() &&
                     ((what_to_skip == ' ' && m_text.getString()[m_cursorPos] == ' ')
                   || (what_to_skip != ' ' && m_text.getString()[m_cursorPos] != ' '))); // sorry, the extra () noise is to shut GCC up (-Wparentheses)
        }
        else if (!key.shift && m_selection && getCursorPos() == m_selection.lower())
        {
            // This is a special case for better ergonomics:
            // Resume the disappearing selection, and move the cursor to the right of it,
            // because what's actually desired is almost never to finish a selection,
            // do nothing with it, and then move the cursor a bit just to kill it...
            m_selection.resume();
            m_selection.set_from_to(getCursorPos(), m_selection.upper());
            setCursorPos(m_selection.upper());
            m_selection.stop();
        }
        else
        {
            // Just move the cursor normally
            setCursorPos(m_cursorPos + 1);
        }
        break;

    case sf::Keyboard::Backspace:
        if (m_selection)
        {
            deleteSelectedText();
            break;
        }
        // Erase character before cursor
        if (m_cursorPos > 0)
        {
            sf::String string = m_text.getString();
            string.erase(m_cursorPos - 1);
            m_text.setString(string);

            setCursorPos(m_cursorPos - 1);
        }
        break;

    case sf::Keyboard::Delete:
        if (m_selection)
        {
            deleteSelectedText();
            break;
        }
        // Erase character after cursor
        if (m_cursorPos < getTextLength())
        {
            sf::String string = m_text.getString();
            string.erase(m_cursorPos);
            m_text.setString(string);

            setCursorPos(m_cursorPos);
        }
        break;

    case sf::Keyboard::Home:
    case sf::Keyboard::Up:
        setCursorPos(0);
        break;

    case sf::Keyboard::End:
    case sf::Keyboard::Down:
        setCursorPos(getTextLength());
        break;

    case sf::Keyboard::Enter:
        triggerCallback();
        break;

    // Ctrl+A: select all
    case sf::Keyboard::A:
        if (key.control)
        {
            setSelection(0, getTextLength());
        }
        break;

    // Ctrl+V: paste clipboard
    case sf::Keyboard::V:
        if (key.control)
        {
            // Delete selected text and write clipboard string over it.
            deleteSelectedText();
            sf::String string = m_text.getString();
            sf::String clipboardString = sf::Clipboard::getString();
            // Trim clipboard content if needed
            if ((string.getSize() + clipboardString.getSize()) > m_maxLength)
            {
                clipboardString = clipboardString.substring(0, m_maxLength - string.getSize());
            }
            // Insert string at cursor position
            string.insert(m_cursorPos, clipboardString);
            m_text.setString(string);
            setCursorPos(m_cursorPos + clipboardString.getSize());
        }
        break;

    // Ctrl+C: copy selected text to clipboard
    case sf::Keyboard::C:
        if (key.control)
        {
            if (m_selection)
            {
                sf::Clipboard::setString(getSelectedText());
            }
        }
        break;

    // Ctrl+X: cut selected text to clipboard
    case sf::Keyboard::X:
        if (key.control)
        {
            if (m_selection)
            {
                sf::Clipboard::setString(getSelectedText());
                deleteSelectedText();
            }
        }
        break;

    default:
        break;
    }
}


void TextBox::onMouseEnter()
{
    assert(getMain());
    getMain()->setMouseCursor(sf::Cursor::Text);
}


void TextBox::onMouseLeave()
{
    assert(getMain());
    getMain()->setMouseCursor(sf::Cursor::Arrow);
}


void TextBox::onMousePressed(float x, float)
{
    size_t pos;
    for (pos = getTextLength(); pos > 0; --pos)
    {
        // Place the cursor after the character under the mouse
        sf::Vector2f glyphPos = m_text.findCharacterPos(pos);
        if (glyphPos.x <= x)
        {
            break;
        }
    }
    setCursorPos(pos);
    m_selection.start(pos); // This looks a bit too eager here: shouldn't
                            // start selecting just by a click, but
                            // a) must record the start pos in case it's
                            //    indeed gonna be a selection, and
                            // b) mouse-release will stop it later anyway
                            //    (as empty if not moved), so no harm done...
}


void TextBox::onMouseReleased(float, float)
{
    m_selection.stop();
}


void TextBox::onMouseMoved(float x, float)
{
    if (getState() != WidgetState::Focused)
        return;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        size_t pos;
        if (x < Theme::borderSize + Theme::PADDING)
        {
            pos = 0;
        }
        else for (pos = getTextLength(); pos > 0; --pos)
        {
            // Place cursor after the character under the mouse
            sf::Vector2f glyphPos = m_text.findCharacterPos(pos);
            if (glyphPos.x <= x)
            {
                break;
            }
        }
        setCursorPos(pos);
    }
}


void TextBox::onTextEntered(char32_t unichar)
{
    if (unichar > 30 && (unichar < 127 || unichar > 159))
    {
        // Delete selected text when a new input is received
        deleteSelectedText();
        sf::String string = m_text.getString();
        if (string.getSize() < m_maxLength)
        {
            // Insert character in string at cursor position
            string.insert(m_cursorPos, unichar);
            m_text.setString(string);
            setCursorPos(m_cursorPos + 1);
        }
    }
}


void TextBox::onStateChanged(WidgetState state)
{
    m_box.applyState(state);

    // Discard selection when focus is lost
    if (state != WidgetState::Focused)
    {
        clearSelection();
    }
}


void TextBox::onThemeChanged()
{
    float offset = Theme::borderSize + Theme::PADDING;

//!! The repositionings below are incomplete alone! x needs readjusting, too!
//!! Ideally no repos. should even be needed, but a clean, net inner rect
//!! should be used instead for all the text + cursor drawing, but... later.
//!!
//!! Alternatively, before draw() (or latest: right there, but that
//!! should of course be avoided for perf. reasons) there should be an
//!! internal update_view() call to iron out any possible inconsistencies
//!! (irrespective of what has caused it)! And it would be called from
//!! onResize, too, in case that becomes a thing (likely for a multi-line
//!! TextBox in the future).

    m_text.setFont(Theme::getFont());
    m_text.setFillColor(Theme::input.textColor);
    m_text.setCharacterSize((unsigned)Theme::textSize);

    m_placeholder.setFont(Theme::getFont());
    m_placeholder.setFillColor(Theme::input.textPlaceholderColor);
    m_placeholder.setCharacterSize((unsigned)Theme::textSize);
    //!! This is a "static fixture", can't move, so it's *probably* OK to
    //!! reposition it only once per theme change:
    m_placeholder.setPosition({offset, offset});

    m_cursorColor = Theme::input.textColor;
    m_cursorRect.setFillColor(Theme::input.textColor);
    //!! Insert/Overwrite would change it too, so this is not future-proof here at all:
    m_cursorRect.setSize(sf::Vector2f(1.f, (float)Theme::getLineSpacing()));

    m_box.setSize(m_width, Theme::getBoxHeight());

    setSize(m_box.getSize());

//!!update_view():
    //!!And then this should adjust the x offset, too (later)!
    m_text.setPosition({m_text.getPosition().x, offset});
    m_cursorRect.setPosition({m_cursorRect.getPosition().x, offset});
}


//----------------------------------------------------------------------------
void TextBox::draw(const gfx::RenderContext& ctx) const
{
    auto sfml_renderstates = ctx.props;
    sfml_renderstates.transform *= getTransform();
    ctx.target.draw(m_box, sfml_renderstates);

    // Crop the text with GL Scissor
    glEnable(GL_SCISSOR_TEST);

    sf::Vector2f pos = getAbsolutePosition();
    auto width = max(0.f, getSize().x - 2 * Theme::borderSize - 2 * Theme::PADDING); // glScissor will fail if < 0!
    glScissor(
        (GLint)(pos.x + Theme::borderSize + Theme::PADDING),
        (GLint)(ctx.target.getSize().y - (pos.y + getSize().y)),
        (GLsizei)width,
        (GLsizei)getSize().y
    );

    if (m_text.getString().isEmpty())
    {
        ctx.target.draw(m_placeholder, sfml_renderstates);
    }
    else
    {
        // Draw the selection indicator
        if (m_selection)
        {
            sf::RectangleShape selRect;
            const sf::Vector2f& startPos = m_text.findCharacterPos(m_selection.lower());
            selRect.setPosition(startPos);
            selRect.setSize({m_text.findCharacterPos(m_selection.upper()).x - startPos.x, m_cursorRect.getSize().y});
            selRect.setFillColor(Theme::input.textSelectionColor);
            ctx.target.draw(selRect, sfml_renderstates);
        }
        ctx.target.draw(m_text, sfml_renderstates);
    }

    glDisable(GL_SCISSOR_TEST);

    // Show cursor if focused
    if (isFocused())
    {
        // Make it blink
        // Hijacking draw() as a timer tick callback... :) Hi five to Alexandre@upstream for the brilliant idea!
        float timer = m_cursorTimer.getElapsedTime().asSeconds();
        if (timer >= m_cursorBlinkPeriod)
            m_cursorTimer.restart();

        m_cursorColor.a = (m_cursorStyle == PULSE ? uint8_t(255 - (255 * timer / m_cursorBlinkPeriod))
                                                  : uint8_t(255 - (255 * timer / m_cursorBlinkPeriod)) & 128 ? 255 : 0);
        m_cursorRect.setFillColor(m_cursorColor);
        ctx.target.draw(m_cursorRect, sfml_renderstates);
    }
}


void TextBox::setSelection(size_t from, size_t length)
{
    m_selection.set_span(from, (int)length);
}

void TextBox::clearSelection()
{
    m_selection.cancel();
}


sf::String TextBox::getSelectedText() const
{
    return m_text.getString().substring(m_selection.lower(), m_selection.length());
}


void TextBox::deleteSelectedText()
{
    // Delete the selected text, if any
    if (!m_selection.empty())
    {
        sf::String str = m_text.getString();
        str.erase(m_selection.lower(), m_selection.length());
        setCursorPos(m_selection.lower());
        m_text.setString(str);
    }
    clearSelection(); //! Must clear it even if empty, so it won't start growing "out of nothing"! (-> #159)
}


TextBox*  TextBox::setPlaceholder(const sf::String& placeholder)
{
    m_placeholder.setString(placeholder);

    return this;
}


const sf::String& TextBox::getPlaceholder() const
{
    return m_placeholder.getString();
}


TextBox* TextBox::setCallback(std::function<void(TextBox*)> callback)
{
    return (TextBox*) Widget::setCallback( [callback] (Widget* w) { callback( (TextBox*)w ); });
}

} // namespace
