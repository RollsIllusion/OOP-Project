// Suppress MSVC deprecation warnings for fopen/fgets (C4996).
// _CRT_SECURE_NO_WARNINGS must appear before any #include.
#define _CRT_SECURE_NO_WARNINGS

// ============================================================
// gui.cpp
// SFML 2.6.2 graphical front-end for MediCore Hospital
// Management System.
//
// Implements every class declared in gui.h.
// No std::string, no global variables, no std::vector.
// All text buffers are plain char arrays.
// All file I/O goes through FileHandler.
// All validation goes through Validator.
// ============================================================

#include "gui.h"
#include "FileHandler.h"
#include "Validator.h"
#include "Utility.h"
#include "DateTime.h"
#include "InsufficientFundsException.h"
#include "InvalidInputException.h"
#include "SlotUnavailableException.h"
#include <cstdio>   // fopen/fgets for security log reading
#include <cstring>  // memset only – no strcmp/strcpy/strtok used

// ============================================================
//  Internal helper: manual safe char copy (replaces strncpy)
// ============================================================
static void safeCopy(char* dst, const char* src, int maxLen) {
    int i = 0;
    while (i < maxLen - 1 && src[i] != '\0') {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\0';
}

// Internal helper: manual char-by-char compare (replaces strcmp)
static bool safeEqual(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return false;
        ++i;
    }
    return a[i] == '\0' && b[i] == '\0';
}

// Internal helper: manual int-to-char (no sprintf/itoa)
static void intToBuf(int v, char* buf) {
    if (v == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    bool neg = v < 0;
    if (neg) v = -v;
    char tmp[20];
    int  len = 0;
    while (v > 0) { tmp[len++] = '0' + (v % 10); v /= 10; }
    if (neg) tmp[len++] = '-';
    int out = 0;
    for (int i = len - 1; i >= 0; --i) buf[out++] = tmp[i];
    buf[out] = '\0';
}

// Internal helper: manual float-to-char (2 decimal places)
static void floatToBuf(float v, char* buf) {
    int whole = (int)v;
    int frac = (int)((v - (float)whole) * 100.0f + 0.5f);
    char wb[20], fb[8];
    intToBuf(whole, wb);
    if (frac < 10) { fb[0] = '0'; intToBuf(frac, fb + 1); }
    else             intToBuf(frac, fb);
    int i = 0;
    while (wb[i]) { buf[i] = wb[i]; ++i; }
    buf[i++] = '.';
    int j = 0;
    while (fb[j]) { buf[i++] = fb[j++]; }
    buf[i] = '\0';
}

// Internal: append src onto dst up to maxLen total chars
static void safeAppend(char* dst, const char* src, int maxLen) {
    int i = 0;
    while (dst[i] != '\0') ++i;
    int j = 0;
    while (src[j] != '\0' && i < maxLen - 1) { dst[i++] = src[j++]; }
    dst[i] = '\0';
}

// ============================================================
//  Slot names constant (also used by BookAppointmentPanel)
// ============================================================
const char* BookAppointmentPanel::SLOTS[8] = {
    "09:00","10:00","11:00","12:00","13:00","14:00","15:00","16:00"
};

// ============================================================
//  Button
// ============================================================
Button::Button()
    : theme_(nullptr), hovered_(false), pressed_(false),
    clicked_(false), enabled_(true), prevMouseDown_(false) {
}

Button::Button(const sf::FloatRect& bounds, const char* label,
    const Theme& theme)
    : theme_(&theme), hovered_(false), pressed_(false),
    clicked_(false), enabled_(true), prevMouseDown_(false)
{
    shape_.setSize({ bounds.width, bounds.height });
    shape_.setPosition(bounds.left, bounds.top);
    shape_.setFillColor(theme.buttonNormal);
    shape_.setOutlineThickness(theme.borderThickness);
    shape_.setOutlineColor(theme.panelBorder);

    label_.setFont(theme.bold);
    label_.setCharacterSize(theme.fontSizeButton);
    label_.setFillColor(theme.buttonText);
    label_.setString(label);

    // Centre text in button
    sf::FloatRect tb = label_.getLocalBounds();
    label_.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    label_.setPosition(bounds.left + bounds.width / 2.f,
        bounds.top + bounds.height / 2.f);
}

void Button::update(sf::Vector2f mousePos, bool mouseDown) {
    clicked_ = false;
    if (!enabled_) { prevMouseDown_ = mouseDown; return; }

    hovered_ = shape_.getGlobalBounds().contains(mousePos);
    bool wasDown = prevMouseDown_;
    prevMouseDown_ = mouseDown;

    if (hovered_ && mouseDown)  pressed_ = true;
    if (hovered_ && wasDown && !mouseDown && pressed_) clicked_ = true;
    if (!mouseDown)              pressed_ = false;

    if (!enabled_)          shape_.setFillColor(sf::Color(120, 120, 120));
    else if (pressed_)      shape_.setFillColor(theme_->buttonActive);
    else if (hovered_)      shape_.setFillColor(theme_->buttonHover);
    else                    shape_.setFillColor(theme_->buttonNormal);
}

void Button::draw(sf::RenderTarget& target) const {
    target.draw(shape_);
    target.draw(label_);
}
bool Button::isClicked() const { return clicked_; }

void Button::setBounds(const sf::FloatRect& b) {
    shape_.setSize({ b.width, b.height });
    shape_.setPosition(b.left, b.top);
    sf::FloatRect tb = label_.getLocalBounds();
    label_.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    label_.setPosition(b.left + b.width / 2.f, b.top + b.height / 2.f);
}

void Button::setLabel(const char* lbl) {
    label_.setString(lbl);
    sf::FloatRect b = shape_.getGlobalBounds();
    sf::FloatRect tb = label_.getLocalBounds();
    label_.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    label_.setPosition(b.left + b.width / 2.f, b.top + b.height / 2.f);
}

void Button::setEnabled(bool e) { enabled_ = e; }
bool Button::isEnabled()  const { return enabled_; }
sf::FloatRect Button::getBounds() const { return shape_.getGlobalBounds(); }

// ============================================================
//  InputBox
// ============================================================
InputBox::InputBox()
    : theme_(nullptr), buffer_(nullptr), maxLen_(0), length_(0),
    focused_(false), masked_(false), cursorVisible_(true) {
}

InputBox::InputBox(const sf::FloatRect& bounds, const char* placeholder,
    const Theme& theme, int maxLen, bool masked)
    : theme_(&theme), maxLen_(maxLen), length_(0),
    focused_(false), masked_(masked), cursorVisible_(true)
{
    buffer_ = new char[maxLen_];
    buffer_[0] = '\0';

    shape_.setSize({ bounds.width, bounds.height });
    shape_.setPosition(bounds.left, bounds.top);
    shape_.setFillColor(theme.inputBg);
    shape_.setOutlineThickness(theme.borderThickness);
    shape_.setOutlineColor(theme.inputBorder);

    displayText_.setFont(theme.regular);
    displayText_.setCharacterSize(theme.fontSizeBody);
    displayText_.setFillColor(theme.inputText);
    displayText_.setPosition(bounds.left + theme.padding,
        bounds.top + (bounds.height - theme.fontSizeBody) / 2.f);

    placeholder_.setFont(theme.regular);
    placeholder_.setCharacterSize(theme.fontSizeBody);
    placeholder_.setFillColor(sf::Color(150, 150, 150));
    placeholder_.setString(placeholder);
    placeholder_.setPosition(displayText_.getPosition());

    cursor_.setSize({ 2.f, bounds.height * 0.6f });
    cursor_.setFillColor(theme.inputText);
    cursor_.setPosition(displayText_.getPosition().x,
        bounds.top + bounds.height * 0.2f);
}

InputBox::~InputBox() { delete[] buffer_; }

// ---------------------------------------------------------------
// InputBox copy constructor — deep-copies the heap buffer so that
// assigning an InputBox to a panel member does not share pointers.
// sf::Text / sf::RectangleShape / sf::Clock are value-copied by
// their own copy constructors which SFML provides.
// ---------------------------------------------------------------
InputBox::InputBox(const InputBox& o)
    : shape_(o.shape_), displayText_(o.displayText_),
    placeholder_(o.placeholder_), cursor_(o.cursor_),
    theme_(o.theme_), maxLen_(o.maxLen_), length_(o.length_),
    focused_(o.focused_), masked_(o.masked_),
    cursorVisible_(o.cursorVisible_)
{
    // Allocate a fresh buffer and copy the text content
    buffer_ = new char[maxLen_];
    for (int i = 0; i <= length_; ++i) buffer_[i] = o.buffer_[i];
}

// ---------------------------------------------------------------
// InputBox copy-assignment — same deep-copy logic.
// ---------------------------------------------------------------
InputBox& InputBox::operator=(const InputBox& o) {
    if (this == &o) return *this;

    // Release the old buffer before reallocating
    delete[] buffer_;

    shape_ = o.shape_;
    displayText_ = o.displayText_;
    placeholder_ = o.placeholder_;
    cursor_ = o.cursor_;
    theme_ = o.theme_;
    maxLen_ = o.maxLen_;
    length_ = o.length_;
    focused_ = o.focused_;
    masked_ = o.masked_;
    cursorVisible_ = o.cursorVisible_;

    buffer_ = new char[maxLen_];
    for (int i = 0; i <= length_; ++i) buffer_[i] = o.buffer_[i];

    return *this;
}

void InputBox::handleEvent(const sf::Event& ev) {
    if (!focused_) return;
    if (ev.type == sf::Event::TextEntered) {
        sf::Uint32 c = ev.text.unicode;
        if (c == 8) { // Backspace
            if (length_ > 0) { --length_; buffer_[length_] = '\0'; }
        }
        else if (c >= 32 && c < 127 && length_ < maxLen_ - 1) {
            buffer_[length_++] = (char)c;
            buffer_[length_] = '\0';
        }
        rebuildDisplay();
    }
}

void InputBox::update(sf::Vector2f mousePos, bool mouseClicked) {
    if (mouseClicked) {
        focused_ = shape_.getGlobalBounds().contains(mousePos);
        shape_.setOutlineColor(focused_ ? theme_->inputBorderFocus
            : theme_->inputBorder);
    }
    // Cursor blink every 0.5 s
    if (cursorClock_.getElapsedTime().asSeconds() > 0.5f) {
        cursorVisible_ = !cursorVisible_;
        cursorClock_.restart();
    }
}

void InputBox::draw(sf::RenderTarget& target) const {
    target.draw(shape_);
    if (length_ == 0)
        target.draw(placeholder_);
    else
        target.draw(displayText_);
    if (focused_ && cursorVisible_) {
        // Position cursor after last character
        sf::FloatRect tb = displayText_.getGlobalBounds();
        sf::RectangleShape cur = cursor_;
        cur.setPosition(tb.left + tb.width + 2.f,
            shape_.getGlobalBounds().top +
            shape_.getGlobalBounds().height * 0.2f);
        target.draw(cur);
    }
}

const char* InputBox::getText() const { return buffer_; }
void InputBox::clear() { length_ = 0; buffer_[0] = '\0'; rebuildDisplay(); }
void InputBox::setFocus(bool f) {
    focused_ = f;
    shape_.setOutlineColor(f ? theme_->inputBorderFocus : theme_->inputBorder);
}
bool          InputBox::hasFocus()  const { return focused_; }
sf::FloatRect InputBox::getBounds() const { return shape_.getGlobalBounds(); }

void InputBox::setBounds(const sf::FloatRect& b) {
    shape_.setSize({ b.width, b.height });
    shape_.setPosition(b.left, b.top);
    displayText_.setPosition(b.left + theme_->padding,
        b.top + (b.height - theme_->fontSizeBody) / 2.f);
    placeholder_.setPosition(displayText_.getPosition());
}

void InputBox::rebuildDisplay() {
    if (masked_) {
        char stars[256];
        int  n = length_ < 255 ? length_ : 255;
        for (int i = 0; i < n; ++i) stars[i] = '*';
        stars[n] = '\0';
        displayText_.setString(stars);
    }
    else {
        displayText_.setString(buffer_);
    }
}

// ============================================================
//  Label
// ============================================================
Label::Label() : theme_(nullptr) {}
Label::Label(sf::Vector2f pos, const char* text, const Theme& theme,
    unsigned int fontSize, sf::Color colour, bool bold)
    : theme_(&theme)
{
    text_.setFont(bold ? theme.bold : theme.regular);
    text_.setCharacterSize(fontSize == 0 ? theme.fontSizeBody : fontSize);
    text_.setFillColor(colour == sf::Color::Transparent
        ? theme.labelText : colour);
    text_.setString(text);
    text_.setPosition(pos);
}
void Label::draw(sf::RenderTarget& t) const { t.draw(text_); }
void Label::setText(const char* s) { text_.setString(s); }
void Label::setPosition(sf::Vector2f p) { text_.setPosition(p); }
void Label::setColour(sf::Color c) { text_.setFillColor(c); }

// ============================================================
//  ScrollableList
// ============================================================
ScrollableList::ScrollableList()
    : theme_(nullptr), rows_(nullptr), isHeader_(nullptr),
    maxRows_(0), rowCount_(0), selectedRow_(-1), scrollOffset_(0),
    rowHeight_(24.f) {
}

ScrollableList::ScrollableList(const sf::FloatRect& bounds,
    const Theme& theme, int maxRows)
    : bounds_(bounds), theme_(&theme), maxRows_(maxRows),
    rowCount_(0), selectedRow_(-1), scrollOffset_(0)
{
    rowHeight_ = (float)(theme.fontSizeBody + (int)theme.padding);

    rows_ = new char* [maxRows_];
    isHeader_ = new bool[maxRows_];
    for (int i = 0; i < maxRows_; ++i) {
        rows_[i] = new char[256];
        rows_[i][0] = '\0';
        isHeader_[i] = false;
    }

    background_.setSize({ bounds.width, bounds.height });
    background_.setPosition(bounds.left, bounds.top);
    background_.setFillColor(theme.panelBg);
    background_.setOutlineThickness(theme.borderThickness);
    background_.setOutlineColor(theme.panelBorder);

    scrollBarShape_.setFillColor(theme.scrollBar);
}

ScrollableList::~ScrollableList() {
    if (rows_) {
        for (int i = 0; i < maxRows_; ++i) delete[] rows_[i];
        delete[] rows_;
    }
    delete[] isHeader_;
}

void ScrollableList::clear() {
    for (int i = 0; i < rowCount_; ++i) rows_[i][0] = '\0';
    rowCount_ = 0; selectedRow_ = -1; scrollOffset_ = 0;
}

void ScrollableList::addRow(const char* text, bool isHeader) {
    if (rowCount_ >= maxRows_) return;
    safeCopy(rows_[rowCount_], text, 256);
    isHeader_[rowCount_] = isHeader;
    ++rowCount_;
}

int ScrollableList::visibleRowCount() const {
    return (int)(bounds_.height / rowHeight_);
}

void ScrollableList::updateScrollBar() {
    if (rowCount_ == 0) return;
    float ratio = (float)visibleRowCount() / (float)rowCount_;
    float barH = bounds_.height * ratio;
    float barY = bounds_.top + ((float)scrollOffset_ / (float)rowCount_) * bounds_.height;
    scrollBarShape_.setSize({ 6.f, barH });
    scrollBarShape_.setPosition(bounds_.left + bounds_.width - 8.f, barY);
}

void ScrollableList::handleEvent(const sf::Event& ev) {
    if (ev.type == sf::Event::MouseWheelScrolled &&
        bounds_.contains((float)ev.mouseWheelScroll.x,
            (float)ev.mouseWheelScroll.y)) {
        scrollOffset_ -= (int)ev.mouseWheelScroll.delta;
        int maxOff = rowCount_ - visibleRowCount();
        if (scrollOffset_ < 0) scrollOffset_ = 0;
        if (scrollOffset_ > maxOff && maxOff > 0) scrollOffset_ = maxOff;
    }
}

void ScrollableList::update(sf::Vector2f mousePos, bool mouseClicked) {
    if (mouseClicked && bounds_.contains(mousePos)) {
        int row = (int)((mousePos.y - bounds_.top) / rowHeight_) + scrollOffset_;
        if (row >= 0 && row < rowCount_) selectedRow_ = row;
    }
    updateScrollBar();
}

void ScrollableList::draw(sf::RenderTarget& target) const {
    target.draw(background_);

    int visible = visibleRowCount();
    for (int i = 0; i < visible; ++i) {
        int idx = i + scrollOffset_;
        if (idx >= rowCount_) break;

        sf::RectangleShape rowBg;
        rowBg.setSize({ bounds_.width, rowHeight_ });
        rowBg.setPosition(bounds_.left, bounds_.top + i * rowHeight_);

        if (isHeader_[idx])
            rowBg.setFillColor(theme_->tableHeader);
        else if (idx == selectedRow_)
            rowBg.setFillColor(theme_->highlight);
        else if (idx % 2 == 0)
            rowBg.setFillColor(theme_->tableRowEven);
        else
            rowBg.setFillColor(theme_->tableRowOdd);
        target.draw(rowBg);

        sf::Text txt;
        txt.setFont(isHeader_[idx] ? theme_->bold : theme_->regular);
        txt.setCharacterSize(isHeader_[idx] ? theme_->fontSizeSmall : theme_->fontSizeBody);
        txt.setFillColor(isHeader_[idx] ? theme_->headingText : theme_->tableText);
        txt.setString(rows_[idx]);
        txt.setPosition(bounds_.left + theme_->padding,
            bounds_.top + i * rowHeight_ + 2.f);
        target.draw(txt);
    }
    if (rowCount_ > visibleRowCount())
        target.draw(scrollBarShape_);
}

int         ScrollableList::getSelectedRow() const { return selectedRow_; }
int         ScrollableList::rowCount()       const { return rowCount_; }
const char* ScrollableList::getRowText(int i) const {
    if (i < 0 || i >= rowCount_) return "";
    return rows_[i];
}

// ============================================================
//  GUI
// ============================================================
GUI::GUI(Storage<Patient>* patients,
    Storage<Doctor>* doctors,
    Storage<Appointment>* appointments,
    Storage<Bill>* bills,
    Storage<Prescription>* prescriptions,
    Admin& admin)
    : window_(nullptr),
    windowSize_(1280, 800),
    patients_(patients),
    doctors_(doctors),
    appointments_(appointments),
    bills_(bills),
    prescriptions_(prescriptions),
    admin_(&admin),
    loggedInPatient_(nullptr),
    loggedInDoctor_(nullptr),
    failedLogins_(0),
    currentScreen_(Screen::RoleSelect),
    messageReturnTo_(Screen::RoleSelect)
{
    for (int i = 0; i < 64; ++i) panels_[i] = nullptr;
}

GUI::~GUI() {
    destroyPanels();
    delete window_;
}

void GUI::initTheme() {
    // Colours — a clean blue/white medical palette
    theme_->background = sf::Color(240, 245, 250);
    theme_->panelBg = sf::Color(255, 255, 255);
    theme_->panelBorder = sf::Color(200, 210, 225);
    theme_->buttonNormal = sf::Color(41, 128, 185);
    theme_->buttonHover = sf::Color(52, 152, 219);
    theme_->buttonActive = sf::Color(31, 97, 141);
    theme_->buttonText = sf::Color(255, 255, 255);
    theme_->inputBg = sf::Color(250, 252, 255);
    theme_->inputBorder = sf::Color(180, 195, 215);
    theme_->inputBorderFocus = sf::Color(41, 128, 185);
    theme_->inputText = sf::Color(30, 30, 30);
    theme_->labelText = sf::Color(60, 60, 80);
    theme_->headingText = sf::Color(20, 60, 110);
    theme_->errorText = sf::Color(192, 57, 43);
    theme_->successText = sf::Color(39, 174, 96);
    theme_->tableHeader = sf::Color(41, 128, 185);
    theme_->tableRowEven = sf::Color(245, 248, 252);
    theme_->tableRowOdd = sf::Color(255, 255, 255);
    theme_->tableText = sf::Color(30, 30, 30);
    theme_->scrollBar = sf::Color(150, 180, 210, 200);
    theme_->highlight = sf::Color(174, 214, 241);

    // Load fonts — use SFML built-in if custom files not found
    // Try loading a system font; fall back gracefully
    if (!theme_->regular.loadFromFile("arial.ttf"))
        theme_->regular.loadFromFile("C:/Windows/Fonts/arial.ttf");
    if (!theme_->bold.loadFromFile("arialbd.ttf"))
        if (!theme_->bold.loadFromFile("C:/Windows/Fonts/arialbd.ttf"))
            theme_->bold = theme_->regular;

    // Sizes
    theme_->fontSizeBody = 14;
    theme_->fontSizeHeading = 22;
    theme_->fontSizeSmall = 12;
    theme_->fontSizeButton = 14;
    theme_->buttonRadius = 4.f;
    theme_->padding = 10.f;
    theme_->inputHeight = 36.f;
    theme_->buttonHeight = 40.f;
    theme_->borderThickness = 1.f;
}

void GUI::initPanels() {
    auto idx = [](Screen s) { return (int)s; };

    panels_[idx(Screen::RoleSelect)] = new RoleSelectPanel(this, theme_);
    panels_[idx(Screen::PatientLogin)] = new LoginPanel(this, theme_, "Patient");
    panels_[idx(Screen::DoctorLogin)] = new LoginPanel(this, theme_, "Doctor");
    panels_[idx(Screen::AdminLogin)] = new LoginPanel(this, theme_, "Admin");
    panels_[idx(Screen::PatientMenu)] = new PatientMenuPanel(this, theme_);
    panels_[idx(Screen::BookAppointment)] = new BookAppointmentPanel(this, theme_);
    panels_[idx(Screen::CancelAppointment)] = new CancelAppointmentPanel(this, theme_);
    panels_[idx(Screen::ViewMyAppointments)] = new ViewAppointmentsPanel(this, theme_);
    panels_[idx(Screen::ViewMedicalRecords)] = new ViewMedicalRecordsPanel(this, theme_);
    panels_[idx(Screen::ViewMyBills)] = new ViewBillsPanel(this, theme_);
    panels_[idx(Screen::PayBill)] = new PayBillPanel(this, theme_);
    panels_[idx(Screen::TopUpBalance)] = new TopUpPanel(this, theme_);
    panels_[idx(Screen::DoctorMenu)] = new DoctorMenuPanel(this, theme_);
    panels_[idx(Screen::ViewToday)] = new ViewTodayPanel(this, theme_);
    panels_[idx(Screen::MarkCompleted)] = new MarkCompletedPanel(this, theme_);
    panels_[idx(Screen::MarkNoShow)] = new MarkNoShowPanel(this, theme_);
    panels_[idx(Screen::WritePrescription)] = new WritePrescriptionPanel(this, theme_);
    panels_[idx(Screen::ViewPatientHistory)] = new ViewPatientHistoryPanel(this, theme_);
    panels_[idx(Screen::AdminMenu)] = new AdminMenuPanel(this, theme_);
    panels_[idx(Screen::AddDoctor)] = new AddDoctorPanel(this, theme_);
    panels_[idx(Screen::RemoveDoctor)] = new RemoveDoctorPanel(this, theme_);
    panels_[idx(Screen::ViewAllPatients)] = new ViewAllPatientsPanel(this, theme_);
    panels_[idx(Screen::ViewAllDoctors)] = new ViewAllDoctorsPanel(this, theme_);
    panels_[idx(Screen::ViewAllAppointments)] = new ViewAllAppointmentsPanel(this, theme_);
    panels_[idx(Screen::ViewUnpaidBills)] = new ViewUnpaidBillsPanel(this, theme_);
    panels_[idx(Screen::DischargePatient)] = new DischargePatientPanel(this, theme_);
    panels_[idx(Screen::SecurityLog)] = new SecurityLogPanel(this, theme_);
    panels_[idx(Screen::DailyReport)] = new DailyReportPanel(this, theme_);
    panels_[idx(Screen::Message)] = new MessagePanel(this, theme_);
}

void GUI::destroyPanels() {
    for (int i = 0; i < 64; ++i) {
        delete panels_[i];
        panels_[i] = nullptr;
    }
}

UIPanel* GUI::activePanel() const {
    int idx = (int)currentScreen_;
    if (idx >= 0 && idx < 64) return panels_[idx];
    return nullptr;
}

void GUI::run() {
    window_ = new sf::RenderWindow(
        sf::VideoMode(windowSize_.x, windowSize_.y),
        "MediCore Hospital Management System",
        sf::Style::Close | sf::Style::Titlebar);
    window_->setFramerateLimit(60);

    initTheme();
    initPanels();

    // Activate the first screen
    currentScreen_ = Screen::RoleSelect;
    if (panels_[(int)Screen::RoleSelect])
        panels_[(int)Screen::RoleSelect]->onEnter();

    sf::Clock clock;
    while (window_->isOpen()) {
        // Exit screen = close window cleanly
        if (currentScreen_ == Screen::Exit) {
            window_->close();
            break;
        }

        float dt = clock.restart().asSeconds();

        // 1. Process OS events + forward to active panel
        processEvents();
        if (!window_->isOpen()) break;

        // 2. Let the active panel update its logic
        update(dt);

        // 3. Check if the panel wants to transition to a new screen
        UIPanel* ap = activePanel();
        if (ap) {
            Screen desired = ap->nextScreen();
            if (desired != currentScreen_) {
                switchTo(desired);
            }
        }

        // 4. Render
        render();
    }
}

void GUI::processEvents() {
    sf::Event ev;
    // Get current real-time mouse state
    bool mouseDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    sf::Vector2f mpos(
        (float)sf::Mouse::getPosition(*window_).x,
        (float)sf::Mouse::getPosition(*window_).y
    );

    while (window_->pollEvent(ev)) {
        // Close window immediately on [X] press
        if (ev.type == sf::Event::Closed) {
            window_->close();
            return;
        }

        // Determine whether this specific event is a left-button release
        bool mouseClicked = (ev.type == sf::Event::MouseButtonReleased &&
            ev.mouseButton.button == sf::Mouse::Left);

        // Delegate every event to the active panel
        UIPanel* ap = activePanel();
        if (ap) ap->handleEvent(ev, mpos, mouseClicked, mouseDown);
    }
}

void GUI::update(float dt) {
    (void)dt;
    bool mouseDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    sf::Vector2f mpos((float)sf::Mouse::getPosition(*window_).x,
        (float)sf::Mouse::getPosition(*window_).y);
    UIPanel* ap = activePanel();
    if (ap) ap->update(0.016f);
    (void)mpos; (void)mouseDown;
}

void GUI::render() {
    window_->clear(theme_->background);
    UIPanel* ap = activePanel();
    if (ap) ap->draw(*window_);
    window_->display();
}

void GUI::switchTo(Screen s) {
    if (s == currentScreen_) return;
    UIPanel* old = activePanel();
    if (old) old->onExit();
    currentScreen_ = s;
    UIPanel* nw = activePanel();
    if (nw) nw->onEnter();
}

void GUI::showMessage(const char* message, Screen returnTo, bool isError) {
    MessagePanel* mp = dynamic_cast<MessagePanel*>(panels_[(int)Screen::Message]);
    if (mp) mp->setMessage(message, returnTo, isError);
    messageReturnTo_ = returnTo;
    switchTo(Screen::Message);
}

Storage<Patient>* GUI::getPatients()      const { return patients_; }
Storage<Doctor>* GUI::getDoctors()        const { return doctors_; }
Storage<Appointment>* GUI::getAppointments()   const { return appointments_; }
Storage<Bill>* GUI::getBills()          const { return bills_; }
Storage<Prescription>* GUI::getPrescriptions()  const { return prescriptions_; }
Admin& GUI::getAdmin()          const { return *admin_; }
const Theme& GUI::getTheme()          const { return theme_; }

void    GUI::setLoggedInPatient(Patient* p) { loggedInPatient_ = p; }
void    GUI::setLoggedInDoctor(Doctor* d) { loggedInDoctor_ = d; }
Patient* GUI::getLoggedInPatient() const { return loggedInPatient_; }
Doctor* GUI::getLoggedInDoctor()  const { return loggedInDoctor_; }

void GUI::clearSession() {
    loggedInPatient_ = nullptr;
    loggedInDoctor_ = nullptr;
    failedLogins_ = 0;
}

void GUI::incrementFailedLogins(const char* role, const char* enteredId) {
    ++failedLogins_;
    FileHandler::logFailedLogin(role, enteredId);
}
void GUI::resetFailedLogins() { failedLogins_ = 0; }
int  GUI::failedLoginCount() const { return failedLogins_; }

// ============================================================
// Helper: build a standard content panel rectangle
// ============================================================
static sf::RectangleShape makePanel(float x, float y, float w, float h,
    const Theme& t) {
    sf::RectangleShape p;
    p.setSize({ w, h });
    p.setPosition(x, y);
    p.setFillColor(t.panelBg);
    p.setOutlineThickness(t.borderThickness);
    p.setOutlineColor(t.panelBorder);
    return p;
}

// Helper: build a heading sf::Text
static sf::Text makeHeading(const char* s, const Theme& t,
    float x, float y) {
    sf::Text txt;
    txt.setFont(t.bold);
    txt.setCharacterSize(t.fontSizeHeading);
    txt.setFillColor(t.headingText);
    txt.setString(s);
    txt.setPosition(x, y);
    return txt;
}

// ============================================================
//  RoleSelectPanel
// ============================================================
RoleSelectPanel::RoleSelectPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::RoleSelect)
{
    float cx = 640.f;
    float cy = 400.f;
    float bw = 280.f, bh = theme.buttonHeight + 10.f;
    float gap = 18.f;
    float startY = cy - 2.f * (bh + gap);

    logo_.setFont(theme.bold);
    logo_.setCharacterSize(36);
    logo_.setFillColor(theme.headingText);
    logo_.setString("MediCore");
    {
        sf::FloatRect lb = logo_.getLocalBounds();
        logo_.setOrigin(lb.left + lb.width / 2.f, 0.f);
        logo_.setPosition(cx, 160.f);
    }

    subtitle_.setFont(theme.regular);
    subtitle_.setCharacterSize(16);
    subtitle_.setFillColor(sf::Color(100, 120, 150));
    subtitle_.setString("Hospital Management System");
    {
        sf::FloatRect lb = subtitle_.getLocalBounds();
        subtitle_.setOrigin(lb.left + lb.width / 2.f, 0.f);
        subtitle_.setPosition(cx, 205.f);
    }

    auto makeBtn = [&](Button& btn, const char* lbl, int i) {
        float y = startY + i * (bh + gap);
        btn = Button({ cx - bw / 2.f, y, bw, bh }, lbl, theme);
        };
    makeBtn(btnPatient_, "Login as Patient", 0);
    makeBtn(btnDoctor_, "Login as Doctor", 1);
    makeBtn(btnAdmin_, "Login as Admin", 2);
    makeBtn(btnExit_, "Exit", 3);

    bgTop_.setSize({ 1280.f, 800.f });
    bgTop_.setPosition(0, 0);
    bgTop_.setFillColor(sf::Color(230, 240, 252));
}

RoleSelectPanel::~RoleSelectPanel() {}

void RoleSelectPanel::onEnter() {
    next_ = Screen::RoleSelect;
    gui_->clearSession();
}

void RoleSelectPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    btnPatient_.update(mpos, mouseDown);
    btnDoctor_.update(mpos, mouseDown);
    btnAdmin_.update(mpos, mouseDown);
    btnExit_.update(mpos, mouseDown);
    (void)ev; (void)clicked;

    if (btnPatient_.isClicked()) next_ = Screen::PatientLogin;
    if (btnDoctor_.isClicked()) next_ = Screen::DoctorLogin;
    if (btnAdmin_.isClicked()) next_ = Screen::AdminLogin;
    if (btnExit_.isClicked()) next_ = Screen::Exit;
}

void RoleSelectPanel::update(float) {}

void RoleSelectPanel::draw(sf::RenderTarget& t) const {
    t.draw(bgTop_);
    t.draw(logo_);
    t.draw(subtitle_);
    btnPatient_.draw(t); btnDoctor_.draw(t);
    btnAdmin_.draw(t); btnExit_.draw(t);
}

Screen RoleSelectPanel::nextScreen() const { return next_; }

// ============================================================
//  LoginPanel
// ============================================================
LoginPanel::LoginPanel(GUI* gui, const Theme& theme, const char* role)
    : gui_(gui), theme_(&theme), next_(Screen::RoleSelect)
{
    safeCopy(role_, role, 16);

    float pw = 420.f, ph = 380.f;
    float px = (1280.f - pw) / 2.f, py = (800.f - ph) / 2.f;

    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading(role, theme, px + pw / 2.f, py + 20.f);
    {
        sf::FloatRect hb = heading_.getLocalBounds();
        heading_.setOrigin(hb.left + hb.width / 2.f, 0.f);
    }

    errorMsg_.setFont(theme.regular);
    errorMsg_.setCharacterSize(theme.fontSizeSmall);
    errorMsg_.setFillColor(theme.errorText);
    errorMsg_.setPosition(px + 20.f, py + 65.f);

    float fy = py + 90.f;
    float fh = theme.inputHeight;

    lblId_ = Label({ px + 20.f, fy }, "ID:", theme);
    inputId_ = InputBox({ px + 20.f, fy + 22.f, pw - 40.f, fh }, "", theme, 20);
    lblPassword_ = Label({ px + 20.f, fy + 70.f }, "Password:", theme);
    inputPassword_ = InputBox({ px + 20.f, fy + 92.f, pw - 40.f, fh }, "", theme, 50, true);

    btnLogin_ = Button({ px + 20.f, fy + 152.f, pw - 40.f, theme.buttonHeight },
        "Login", theme);
    btnBack_ = Button({ px + 20.f, fy + 204.f, pw - 40.f, theme.buttonHeight },
        "Back", theme);
}

LoginPanel::~LoginPanel() {}

void LoginPanel::onEnter() {
    next_ = selfScreen();
    inputId_.clear();
    inputPassword_.clear();
    errorMsg_.setString("");
    gui_->resetFailedLogins();
}

Screen LoginPanel::successScreen() const {
    if (safeEqual(role_, "Patient")) return Screen::PatientMenu;
    if (safeEqual(role_, "Doctor"))  return Screen::DoctorMenu;
    return Screen::AdminMenu;
}

Screen LoginPanel::selfScreen() const {
    if (safeEqual(role_, "Patient")) return Screen::PatientLogin;
    if (safeEqual(role_, "Doctor"))  return Screen::DoctorLogin;
    return Screen::AdminLogin;
}

void LoginPanel::attemptLogin() {
    const char* idStr = inputId_.getText();
    const char* pw = inputPassword_.getText();
    int id = Validator::charToInt(idStr);

    if (safeEqual(role_, "Patient")) {
        Patient* p = gui_->getPatients()->findById(id);
        if (p && p->checkPassword(pw)) {
            gui_->resetFailedLogins();
            gui_->setLoggedInPatient(p);
            next_ = Screen::PatientMenu;
            return;
        }
    }
    else if (safeEqual(role_, "Doctor")) {
        Doctor* d = gui_->getDoctors()->findById(id);
        if (d && d->checkPassword(pw)) {
            gui_->resetFailedLogins();
            gui_->setLoggedInDoctor(d);
            next_ = Screen::DoctorMenu;
            return;
        }
    }
    else { // Admin
        Admin& admin = gui_->getAdmin();
        if (id == admin.getId() && admin.checkPassword(pw)) {
            gui_->resetFailedLogins();
            next_ = Screen::AdminMenu;
            return;
        }
    }

    // Failure
    gui_->incrementFailedLogins(role_, idStr);
    int remaining = 3 - gui_->failedLoginCount();

    if (remaining <= 0) {
        errorMsg_.setString("Account locked. Contact admin.");
        next_ = Screen::RoleSelect;
    }
    else {
        char msg[80] = "Invalid ID or password. Attempts left: ";
        char nb[4]; intToBuf(remaining, nb);
        safeAppend(msg, nb, 80);
        errorMsg_.setString(msg);
    }
    inputPassword_.clear();
}

void LoginPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    inputId_.handleEvent(ev);
    inputPassword_.handleEvent(ev);
    inputId_.update(mpos, clicked);
    inputPassword_.update(mpos, clicked);
    btnLogin_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);

    if (btnLogin_.isClicked()) attemptLogin();
    if (btnBack_.isClicked()) next_ = Screen::RoleSelect;

    // Enter key submits
    if (ev.type == sf::Event::KeyPressed &&
        ev.key.code == sf::Keyboard::Return)
        attemptLogin();
    (void)clicked;
}

void LoginPanel::update(float) {}

void LoginPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); t.draw(errorMsg_);
    lblId_.draw(t);       inputId_.draw(t);
    lblPassword_.draw(t); inputPassword_.draw(t);
    btnLogin_.draw(t);    btnBack_.draw(t);
}

Screen LoginPanel::nextScreen() const { return next_; }

// ============================================================
//  PatientMenuPanel
// ============================================================
PatientMenuPanel::PatientMenuPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::PatientMenu)
{
    float bw = 380.f, bh = theme.buttonHeight;
    float cx = 640.f, sy = 180.f, gap = 12.f;

    headerBar_.setSize({ 1280.f, 80.f });
    headerBar_.setPosition(0.f, 0.f);
    headerBar_.setFillColor(theme.headingText);

    lblWelcome_.setFont(theme.bold);
    lblWelcome_.setCharacterSize(20);
    lblWelcome_.setFillColor(sf::Color::White);
    lblWelcome_.setPosition(20.f, 22.f);

    lblBalance_.setFont(theme.regular);
    lblBalance_.setCharacterSize(15);
    lblBalance_.setFillColor(sf::Color(200, 230, 255));
    lblBalance_.setPosition(20.f, 52.f);

    auto mkBtn = [&](Button& b, const char* lbl, int row, int col) {
        float x = (col == 0) ? cx - bw - 10.f : cx + 10.f;
        float y = sy + row * (bh + gap);
        b = Button({ x, y, bw, bh }, lbl, theme);
        };

    mkBtn(btnBook_, "1. Book Appointment", 0, 0);
    mkBtn(btnCancel_, "2. Cancel Appointment", 0, 1);
    mkBtn(btnViewAppts_, "3. View My Appointments", 1, 0);
    mkBtn(btnMedRecords_, "4. View Medical Records", 1, 1);
    mkBtn(btnBills_, "5. View My Bills", 2, 0);
    mkBtn(btnPayBill_, "6. Pay Bill", 2, 1);
    mkBtn(btnTopUp_, "7. Top Up Balance", 3, 0);
    btnLogout_ = Button({ cx - bw / 2.f, sy + 4 * (bh + gap), bw, bh },
        "Logout", theme);
}

PatientMenuPanel::~PatientMenuPanel() {}

void PatientMenuPanel::refreshHeader() {
    Patient* p = gui_->getLoggedInPatient();
    if (!p) return;
    char msg[120] = "Welcome, ";
    safeAppend(msg, p->getName(), 120);
    lblWelcome_.setString(msg);

    char bal[40] = "Balance: PKR ";
    char fb[20]; floatToBuf(p->getBalance(), fb);
    safeAppend(bal, fb, 40);
    lblBalance_.setString(bal);
}

void PatientMenuPanel::onEnter() {
    next_ = Screen::PatientMenu;
    refreshHeader();
}

void PatientMenuPanel::handleEvent(const sf::Event&, sf::Vector2f mpos,
    bool, bool mouseDown) {
    btnBook_.update(mpos, mouseDown);
    btnCancel_.update(mpos, mouseDown);
    btnViewAppts_.update(mpos, mouseDown);
    btnMedRecords_.update(mpos, mouseDown);
    btnBills_.update(mpos, mouseDown);
    btnPayBill_.update(mpos, mouseDown);
    btnTopUp_.update(mpos, mouseDown);
    btnLogout_.update(mpos, mouseDown);

    if (btnBook_.isClicked()) next_ = Screen::BookAppointment;
    if (btnCancel_.isClicked()) next_ = Screen::CancelAppointment;
    if (btnViewAppts_.isClicked()) next_ = Screen::ViewMyAppointments;
    if (btnMedRecords_.isClicked()) next_ = Screen::ViewMedicalRecords;
    if (btnBills_.isClicked()) next_ = Screen::ViewMyBills;
    if (btnPayBill_.isClicked()) next_ = Screen::PayBill;
    if (btnTopUp_.isClicked()) next_ = Screen::TopUpBalance;
    if (btnLogout_.isClicked()) { gui_->clearSession(); next_ = Screen::RoleSelect; }
}

void PatientMenuPanel::update(float) { refreshHeader(); }

void PatientMenuPanel::draw(sf::RenderTarget& t) const {
    t.draw(headerBar_); t.draw(lblWelcome_); t.draw(lblBalance_);
    btnBook_.draw(t);  btnCancel_.draw(t);
    btnViewAppts_.draw(t); btnMedRecords_.draw(t);
    btnBills_.draw(t); btnPayBill_.draw(t);
    btnTopUp_.draw(t); btnLogout_.draw(t);
}

Screen PatientMenuPanel::nextScreen() const { return next_; }

// ============================================================
//  BookAppointmentPanel
// ============================================================
BookAppointmentPanel::BookAppointmentPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), step_(0),
    matchedDoctorCount_(0), selectedDoctor_(nullptr),
    dateAttempts_(0), next_(Screen::BookAppointment)
{
    for (int i = 0; i < 100; ++i) matchedDoctorIndices_[i] = 0;
    selectedDate_[0] = '\0'; selectedSlot_[0] = '\0';

    float px = 80.f, py = 80.f, pw = 1120.f, ph = 680.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Book Appointment", theme, px + 20.f, py + 15.f);

    float fy = py + 65.f, fw = pw - 40.f;

    lblSpec_ = Label({ px + 20.f, fy }, "Specialization:", theme);
    inputSpec_ = InputBox({ px + 20.f, fy + 24.f, 400.f, theme.inputHeight }, "e.g. Cardiology", theme, 50);
    btnSearchSpec_ = Button({ px + 440.f, fy + 22.f, 140.f, theme.buttonHeight + 2.f }, "Search", theme);
    btnBack_ = Button({ px + 20.f, py + ph - 60.f, 120.f, theme.buttonHeight }, "Back", theme);

    doctorList_ = ScrollableList({ px + 20.f, fy + 75.f, fw, 200.f }, theme, 100);
    btnSelectDoctor_ = Button({ px + 20.f, fy + 285.f, 200.f, theme.buttonHeight }, "Select Doctor", theme);

    lblDate_ = Label({ px + 20.f, fy }, "Date (DD-MM-YYYY):", theme);
    inputDate_ = InputBox({ px + 20.f, fy + 24.f, 200.f, theme.inputHeight }, "DD-MM-YYYY", theme, 12);
    btnConfirmDate_ = Button({ px + 240.f, fy + 22.f, 120.f, theme.buttonHeight + 2.f }, "Confirm", theme);
    dateError_.setFont(theme.regular);
    dateError_.setCharacterSize(theme.fontSizeSmall);
    dateError_.setFillColor(theme.errorText);
    dateError_.setPosition(px + 20.f, fy + 70.f);

    // Slot buttons — 2 rows of 4
    for (int i = 0; i < 8; ++i) {
        float sx = px + 20.f + (i % 4) * 160.f;
        float sy = fy + 30.f + (i / 4) * 60.f;
        slotButtons_[i] = Button({ sx, sy, 140.f, 44.f }, SLOTS[i], theme);
        slotAvailable_[i] = true;
    }
    slotError_.setFont(theme.regular);
    slotError_.setCharacterSize(theme.fontSizeSmall);
    slotError_.setFillColor(theme.errorText);
    slotError_.setPosition(px + 20.f, fy + 160.f);

    confirmText_.setFont(theme.regular);
    confirmText_.setCharacterSize(theme.fontSizeBody);
    confirmText_.setFillColor(theme.labelText);
    confirmText_.setPosition(px + 20.f, fy);

    btnConfirm_ = Button({ px + 20.f, fy + 140.f, 160.f, theme.buttonHeight }, "Confirm Booking", theme);
    btnCancelBook_ = Button({ px + 200.f, fy + 140.f, 120.f, theme.buttonHeight }, "Cancel", theme);
}

BookAppointmentPanel::~BookAppointmentPanel() {}

void BookAppointmentPanel::onEnter() {
    next_ = Screen::BookAppointment;
    resetToStep(0);
}

void BookAppointmentPanel::resetToStep(int s) {
    step_ = s;
    if (s == 0) {
        inputSpec_.clear();
        doctorList_.clear();
        matchedDoctorCount_ = 0;
        selectedDoctor_ = nullptr;
        selectedDate_[0] = '\0';
        selectedSlot_[0] = '\0';
        dateAttempts_ = 0;
        dateError_.setString("");
    }
}

void BookAppointmentPanel::doSpecSearch() {
    const char* spec = inputSpec_.getText();
    if (Validator::strLen(spec) == 0) return;

    doctorList_.clear();
    matchedDoctorCount_ = 0;

    Storage<Doctor>* docs = gui_->getDoctors();
    Doctor* arr = docs->getAll();
    int n = docs->size();

    for (int i = 0; i < n; ++i) {
        if (Validator::specializationMatch(arr[i].getSpecialization(), spec)) {
            char row[256];
            char fb[20]; floatToBuf(arr[i].getFee(), fb);
            row[0] = '\0';
            char idb[10]; intToBuf(arr[i].getId(), idb);
            safeAppend(row, "ID:", 256); safeAppend(row, idb, 256);
            safeAppend(row, "  ", 256);  safeAppend(row, arr[i].getName(), 256);
            safeAppend(row, "  Fee: PKR ", 256); safeAppend(row, fb, 256);
            doctorList_.addRow(row);
            matchedDoctorIndices_[matchedDoctorCount_++] = i;
        }
    }
    if (matchedDoctorCount_ == 0)
        doctorList_.addRow("No doctors available for that specialization.");
    else
        step_ = 1;
}

void BookAppointmentPanel::buildDoctorList() {}

void BookAppointmentPanel::buildSlotGrid() {
    computeSlotAvailability();
}

bool BookAppointmentPanel::computeSlotAvailability() {
    if (!selectedDoctor_) return false;
    Storage<Appointment>* appts = gui_->getAppointments();
    Appointment* arr = appts->getAll();
    int n = appts->size();

    for (int i = 0; i < 8; ++i) slotAvailable_[i] = true;

    for (int i = 0; i < n; ++i) {
        if (arr[i].getDoctorId() == selectedDoctor_->getId() &&
            safeEqual(arr[i].getDate(), selectedDate_) &&
            !safeEqual(arr[i].getStatus(), "cancelled")) {
            for (int s = 0; s < 8; ++s) {
                if (safeEqual(arr[i].getTimeSlot(), SLOTS[s])) {
                    slotAvailable_[s] = false;
                    break;
                }
            }
        }
    }
    return true;
}

void BookAppointmentPanel::doBooking() {
    Patient* patient = gui_->getLoggedInPatient();
    if (!patient || !selectedDoctor_) return;

    float fee = selectedDoctor_->getFee();
    if (patient->getBalance() < fee) {
        gui_->showMessage("Insufficient balance. Please top up.", Screen::BookAppointment, true);
        resetToStep(0);
        return;
    }

    *patient -= fee;
    int newId = gui_->getAppointments()->getMaxId() + 1;
    Appointment a(newId, patient->getId(), selectedDoctor_->getId(),
        selectedDate_, selectedSlot_, "pending");
    gui_->getAppointments()->add(a);
    FileHandler::appendAppointment(a);

    int billId = gui_->getBills()->getMaxId() + 1;
    Bill b(billId, patient->getId(), newId, fee, "unpaid", selectedDate_);
    gui_->getBills()->add(b);
    FileHandler::appendBill(b);
    FileHandler::updatePatient(*gui_->getPatients());

    char msg[80] = "Appointment booked! ID: ";
    char idb[12]; intToBuf(newId, idb);
    safeAppend(msg, idb, 80);
    gui_->showMessage(msg, Screen::PatientMenu, false);
    resetToStep(0);
}

void BookAppointmentPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked;
    switch (step_) {
    case 0:
        inputSpec_.handleEvent(ev);
        inputSpec_.update(mpos, mouseDown);
        btnSearchSpec_.update(mpos, mouseDown);
        btnBack_.update(mpos, mouseDown);
        if (btnSearchSpec_.isClicked()) doSpecSearch();
        if (btnBack_.isClicked()) next_ = Screen::PatientMenu;
        if (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Return)
            doSpecSearch();
        break;

    case 1:
        doctorList_.handleEvent(ev);
        doctorList_.update(mpos, mouseDown);
        btnSelectDoctor_.update(mpos, mouseDown);
        btnBack_.update(mpos, mouseDown);
        if (btnSelectDoctor_.isClicked()) {
            int sel = doctorList_.getSelectedRow();
            if (sel >= 0 && sel < matchedDoctorCount_) {
                int idx = matchedDoctorIndices_[sel];
                selectedDoctor_ = &gui_->getDoctors()->getAll()[idx];
                step_ = 2;
                inputDate_.clear();
                dateError_.setString("");
            }
        }
        if (btnBack_.isClicked()) resetToStep(0);
        break;

    case 2:
        inputDate_.handleEvent(ev);
        inputDate_.update(mpos, mouseDown);
        btnConfirmDate_.update(mpos, mouseDown);
        btnBack_.update(mpos, mouseDown);
        if (btnConfirmDate_.isClicked() ||
            (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Return)) {
            const char* d = inputDate_.getText();
            if (Validator::validateDate(d, Utility::getCurrentYear())) {
                safeCopy(selectedDate_, d, 12);
                buildSlotGrid();
                step_ = 3;
                slotError_.setString("");
            }
            else {
                ++dateAttempts_;
                if (dateAttempts_ >= 3) {
                    gui_->showMessage("Too many invalid date attempts.", Screen::PatientMenu, true);
                    resetToStep(0);
                }
                else {
                    dateError_.setString("Invalid date. Use DD-MM-YYYY, current year or later.");
                }
            }
        }
        if (btnBack_.isClicked()) { step_ = 1; }
        break;

    case 3:
        for (int i = 0; i < 8; ++i) {
            slotButtons_[i].update(mpos, mouseDown);
            if (slotButtons_[i].isClicked()) {
                if (!slotAvailable_[i]) {
                    slotError_.setString("That slot is unavailable. Please choose another.");
                }
                else {
                    safeCopy(selectedSlot_, SLOTS[i], 6);
                    step_ = 4;
                    // Build confirm text
                    char msg[300] = "Doctor: ";
                    safeAppend(msg, selectedDoctor_->getName(), 300);
                    safeAppend(msg, "\nDate: ", 300);
                    safeAppend(msg, selectedDate_, 300);
                    safeAppend(msg, "\nSlot: ", 300);
                    safeAppend(msg, selectedSlot_, 300);
                    safeAppend(msg, "\nFee: PKR ", 300);
                    char fb[20]; floatToBuf(selectedDoctor_->getFee(), fb);
                    safeAppend(msg, fb, 300);
                    confirmText_.setString(msg);
                }
            }
        }
        btnBack_.update(mpos, mouseDown);
        if (btnBack_.isClicked()) { step_ = 2; }
        break;

    case 4:
        btnConfirm_.update(mpos, mouseDown);
        btnCancelBook_.update(mpos, mouseDown);
        btnBack_.update(mpos, mouseDown);
        if (btnConfirm_.isClicked()) doBooking();
        if (btnCancelBook_.isClicked()) resetToStep(0);
        if (btnBack_.isClicked()) step_ = 3;
        break;
    }
}

void BookAppointmentPanel::update(float) {}

void BookAppointmentPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_);

    float px = 80.f, py = 80.f, fy = py + 65.f;

    switch (step_) {
    case 0:
        lblSpec_.draw(t); inputSpec_.draw(t); btnSearchSpec_.draw(t);
        btnBack_.draw(t);
        break;
    case 1:
        doctorList_.draw(t); btnSelectDoctor_.draw(t); btnBack_.draw(t);
        break;
    case 2:
        lblDate_.draw(t); inputDate_.draw(t); btnConfirmDate_.draw(t);
        t.draw(dateError_); btnBack_.draw(t);
        break;
    case 3: {
        // Show slot buttons; grey out unavailable
        sf::Text slotTitle;
        slotTitle.setFont(theme_->regular);
        slotTitle.setCharacterSize(theme_->fontSizeBody);
        slotTitle.setFillColor(theme_->labelText);
        slotTitle.setString("Select a time slot:");
        slotTitle.setPosition(px + 20.f, fy);
        t.draw(slotTitle);
        for (int i = 0; i < 8; ++i) {
            // Draw coloured overlay for unavailable
            slotButtons_[i].draw(t);
            if (!slotAvailable_[i]) {
                sf::RectangleShape overlay;
                sf::FloatRect b = slotButtons_[i].getBounds();
                overlay.setSize({ b.width, b.height });
                overlay.setPosition(b.left, b.top);
                overlay.setFillColor(sf::Color(200, 80, 80, 180));
                t.draw(overlay);
                sf::Text x;
                x.setFont(theme_->regular);
                x.setCharacterSize(10);
                x.setFillColor(sf::Color::White);
                x.setString("Taken");
                x.setPosition(b.left + 4.f, b.top + b.height - 18.f);
                t.draw(x);
            }
        }
        t.draw(slotError_); btnBack_.draw(t);
        break;
    }
    case 4:
        t.draw(confirmText_);
        btnConfirm_.draw(t); btnCancelBook_.draw(t); btnBack_.draw(t);
        break;
    }
}

Screen BookAppointmentPanel::nextScreen() const { return next_; }

// ============================================================
//  CancelAppointmentPanel
// ============================================================
CancelAppointmentPanel::CancelAppointmentPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), apptCount_(0), next_(Screen::CancelAppointment)
{
    float px = 120.f, py = 80.f, pw = 1040.f, ph = 660.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Cancel Appointment", theme, px + 20.f, py + 15.f);
    statusMsg_.setFont(theme.regular);
    statusMsg_.setCharacterSize(theme.fontSizeSmall);
    statusMsg_.setFillColor(theme.errorText);
    statusMsg_.setPosition(px + 20.f, py + ph - 50.f);

    apptList_ = ScrollableList({ px + 20.f, py + 65.f, pw - 40.f, 440.f }, theme);
    btnCancel_ = Button({ px + 20.f, py + ph - 90.f, 200.f, theme.buttonHeight },
        "Cancel Selected", theme);
    btnBack_ = Button({ px + 240.f, py + ph - 90.f, 120.f, theme.buttonHeight },
        "Back", theme);
}

CancelAppointmentPanel::~CancelAppointmentPanel() {}

void CancelAppointmentPanel::onEnter() {
    next_ = Screen::CancelAppointment;
    statusMsg_.setString("");
    loadPendingAppointments();
}

void CancelAppointmentPanel::loadPendingAppointments() {
    apptList_.clear(); apptCount_ = 0;
    Patient* p = gui_->getLoggedInPatient();
    if (!p) return;

    // Header
    apptList_.addRow("ID    | Doctor Name              | Date       | Slot  | Status", true);

    Appointment* arr = gui_->getAppointments()->getAll();
    int n = gui_->getAppointments()->size();
    for (int i = 0; i < n; ++i) {
        if (arr[i].getPatientId() == p->getId() &&
            safeEqual(arr[i].getStatus(), "pending")) {
            Doctor* d = gui_->getDoctors()->findById(arr[i].getDoctorId());
            const char* dn = d ? d->getName() : "Unknown";
            char row[256];
            char idb[10]; intToBuf(arr[i].getAppointmentId(), idb);
            row[0] = '\0';
            safeAppend(row, idb, 256); safeAppend(row, "  | ", 256);
            safeAppend(row, dn, 256);  safeAppend(row, " | ", 256);
            safeAppend(row, arr[i].getDate(), 256); safeAppend(row, " | ", 256);
            safeAppend(row, arr[i].getTimeSlot(), 256);
            apptList_.addRow(row);
            if (apptCount_ < 100) apptIds_[apptCount_++] = arr[i].getAppointmentId();
        }
    }
    if (apptCount_ == 0) apptList_.addRow("No pending appointments.");
}

void CancelAppointmentPanel::doCancellation() {
    int sel = apptList_.getSelectedRow();
    // row 0 is header
    int listIdx = sel - 1;
    if (sel < 1 || listIdx >= apptCount_) {
        statusMsg_.setString("Please select an appointment to cancel.");
        return;
    }
    int apptId = apptIds_[listIdx];
    Appointment* appt = gui_->getAppointments()->findById(apptId);
    if (!appt || !safeEqual(appt->getStatus(), "pending")) {
        statusMsg_.setString("Invalid appointment.");
        return;
    }

    // Find bill
    Bill* bills = gui_->getBills()->getAll();
    int bn = gui_->getBills()->size();
    float refund = 0.f;
    for (int i = 0; i < bn; ++i) {
        if (bills[i].getAppointmentId() == apptId) {
            refund = bills[i].getAmount();
            bills[i].setStatus("cancelled");
            break;
        }
    }
    appt->setStatus("cancelled");

    Patient* p = gui_->getLoggedInPatient();
    *p += refund;
    FileHandler::updateAppointment(*gui_->getAppointments());
    FileHandler::updateBill(*gui_->getBills());
    FileHandler::updatePatient(*gui_->getPatients());

    char msg[80] = "Cancelled. PKR ";
    char fb[20]; floatToBuf(refund, fb);
    safeAppend(msg, fb, 80); safeAppend(msg, " refunded.", 80);
    gui_->showMessage(msg, Screen::PatientMenu, false);
}

void CancelAppointmentPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked;
    apptList_.handleEvent(ev);
    apptList_.update(mpos, mouseDown);
    btnCancel_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnCancel_.isClicked()) doCancellation();
    if (btnBack_.isClicked()) next_ = Screen::PatientMenu;
}

void CancelAppointmentPanel::update(float) {}

void CancelAppointmentPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); t.draw(statusMsg_);
    apptList_.draw(t); btnCancel_.draw(t); btnBack_.draw(t);
}

Screen CancelAppointmentPanel::nextScreen() const { return next_; }

// ============================================================
//  ViewAppointmentsPanel
// ============================================================
ViewAppointmentsPanel::ViewAppointmentsPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::ViewMyAppointments)
{
    float px = 60.f, py = 70.f, pw = 1160.f, ph = 680.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("My Appointments", theme, px + 20.f, py + 15.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 560.f }, theme);
    btnBack_ = Button({ px + 20.f, py + ph - 55.f, 120.f, theme.buttonHeight }, "Back", theme);
}

ViewAppointmentsPanel::~ViewAppointmentsPanel() {}

void ViewAppointmentsPanel::onEnter() {
    next_ = Screen::ViewMyAppointments;
    loadAppointments();
}

void ViewAppointmentsPanel::loadAppointments() {
    list_.clear();
    list_.addRow("ID  | Doctor                   | Specialization | Date       | Slot  | Status", true);

    Patient* p = gui_->getLoggedInPatient();
    if (!p) return;

    // Collect and sort ascending by date
    Appointment* arr = gui_->getAppointments()->getAll();
    int n = gui_->getAppointments()->size();
    Appointment* filtered[100]; int cnt = 0;
    for (int i = 0; i < n && cnt < 100; ++i)
        if (arr[i].getPatientId() == p->getId()) filtered[cnt++] = &arr[i];

    Utility::sortAppointmentsAsc(filtered, cnt);

    for (int i = 0; i < cnt; ++i) {
        Doctor* d = gui_->getDoctors()->findById(filtered[i]->getDoctorId());
        const char* dn = d ? d->getName() : "Unknown";
        const char* spec = d ? d->getSpecialization() : "-";
        char row[256]; row[0] = '\0';
        char idb[10]; intToBuf(filtered[i]->getAppointmentId(), idb);
        safeAppend(row, idb, 256); safeAppend(row, " | ", 256);
        safeAppend(row, dn, 256); safeAppend(row, " | ", 256);
        safeAppend(row, spec, 256); safeAppend(row, " | ", 256);
        safeAppend(row, filtered[i]->getDate(), 256); safeAppend(row, " | ", 256);
        safeAppend(row, filtered[i]->getTimeSlot(), 256); safeAppend(row, " | ", 256);
        safeAppend(row, filtered[i]->getStatus(), 256);
        list_.addRow(row);
    }
    if (cnt == 0) list_.addRow("No appointments found.");
}

void ViewAppointmentsPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked;
    list_.handleEvent(ev);
    list_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnBack_.isClicked()) next_ = Screen::PatientMenu;
}

void ViewAppointmentsPanel::update(float) {}

void ViewAppointmentsPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); list_.draw(t); btnBack_.draw(t);
}

Screen ViewAppointmentsPanel::nextScreen() const { return next_; }

// ============================================================
//  ViewMedicalRecordsPanel
// ============================================================
ViewMedicalRecordsPanel::ViewMedicalRecordsPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::ViewMedicalRecords)
{
    float px = 60.f, py = 70.f, pw = 1160.f, ph = 680.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("My Medical Records", theme, px + 20.f, py + 15.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 560.f }, theme);
    btnBack_ = Button({ px + 20.f, py + ph - 55.f, 120.f, theme.buttonHeight }, "Back", theme);
}

ViewMedicalRecordsPanel::~ViewMedicalRecordsPanel() {}

void ViewMedicalRecordsPanel::onEnter() {
    next_ = Screen::ViewMedicalRecords;
    loadRecords();
}

void ViewMedicalRecordsPanel::loadRecords() {
    list_.clear();
    Patient* p = gui_->getLoggedInPatient();
    if (!p) return;

    Prescription* arr = gui_->getPrescriptions()->getAll();
    int n = gui_->getPrescriptions()->size();
    Prescription* filtered[100]; int cnt = 0;
    for (int i = 0; i < n && cnt < 100; ++i)
        if (arr[i].getPatientId() == p->getId()) filtered[cnt++] = &arr[i];

    if (cnt == 0) { list_.addRow("No medical records found."); return; }
    Utility::sortPrescriptionsDesc(filtered, cnt);

    for (int i = 0; i < cnt; ++i) {
        Doctor* d = gui_->getDoctors()->findById(filtered[i]->getDoctorId());
        const char* dn = d ? d->getName() : "Unknown";
        // Header row for each record
        char hdr[256] = "Date: ";
        safeAppend(hdr, filtered[i]->getDate(), 256);
        safeAppend(hdr, "  Dr. ", 256); safeAppend(hdr, dn, 256);
        list_.addRow(hdr, true);

        char med[512] = "Medicines: ";
        safeAppend(med, filtered[i]->getMedicines(), 512);
        list_.addRow(med);

        char notes[350] = "Notes: ";
        safeAppend(notes, filtered[i]->getNotes(), 350);
        list_.addRow(notes);
    }
}

void ViewMedicalRecordsPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked;
    list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnBack_.isClicked()) next_ = Screen::PatientMenu;
}

void ViewMedicalRecordsPanel::update(float) {}

void ViewMedicalRecordsPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); list_.draw(t); btnBack_.draw(t);
}

Screen ViewMedicalRecordsPanel::nextScreen() const { return next_; }

// ============================================================
//  ViewBillsPanel
// ============================================================
ViewBillsPanel::ViewBillsPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::ViewMyBills)
{
    float px = 120.f, py = 70.f, pw = 1040.f, ph = 680.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("My Bills", theme, px + 20.f, py + 15.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 520.f }, theme);
    totalOwed_.setFont(theme.bold);
    totalOwed_.setCharacterSize(theme.fontSizeBody);
    totalOwed_.setFillColor(theme.errorText);
    totalOwed_.setPosition(px + 20.f, py + 590.f);
    btnBack_ = Button({ px + 20.f, py + ph - 55.f, 120.f, theme.buttonHeight }, "Back", theme);
}

ViewBillsPanel::~ViewBillsPanel() {}

void ViewBillsPanel::onEnter() { next_ = Screen::ViewMyBills; loadBills(); }

void ViewBillsPanel::loadBills() {
    list_.clear();
    list_.addRow("BillID | ApptID | Amount (PKR) | Status   | Date", true);

    Patient* p = gui_->getLoggedInPatient();
    if (!p) return;

    Bill* arr = gui_->getBills()->getAll();
    int n = gui_->getBills()->size();
    float total = 0.f;
    bool found = false;

    for (int i = 0; i < n; ++i) {
        if (arr[i].getPatientId() != p->getId()) continue;
        found = true;
        char row[256]; row[0] = '\0';
        char bid[10], aid[10], ab[20];
        intToBuf(arr[i].getBillId(), bid);
        intToBuf(arr[i].getAppointmentId(), aid);
        floatToBuf(arr[i].getAmount(), ab);
        safeAppend(row, bid, 256); safeAppend(row, " | ", 256);
        safeAppend(row, aid, 256); safeAppend(row, " | PKR ", 256);
        safeAppend(row, ab, 256); safeAppend(row, " | ", 256);
        safeAppend(row, arr[i].getStatus(), 256); safeAppend(row, " | ", 256);
        safeAppend(row, arr[i].getDate(), 256);
        list_.addRow(row);
        if (safeEqual(arr[i].getStatus(), "unpaid")) total += arr[i].getAmount();
    }
    if (!found) { list_.addRow("No bills found."); return; }

    char ts[60] = "Total outstanding (unpaid): PKR ";
    char tb[20]; floatToBuf(total, tb);
    safeAppend(ts, tb, 60);
    totalOwed_.setString(ts);
}

void ViewBillsPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked;
    list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnBack_.isClicked()) next_ = Screen::PatientMenu;
}

void ViewBillsPanel::update(float) {}

void ViewBillsPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); list_.draw(t);
    t.draw(totalOwed_); btnBack_.draw(t);
}

Screen ViewBillsPanel::nextScreen() const { return next_; }

// ============================================================
//  PayBillPanel
// ============================================================
PayBillPanel::PayBillPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), billCount_(0), next_(Screen::PayBill)
{
    float px = 120.f, py = 80.f, pw = 1040.f, ph = 660.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Pay Bill", theme, px + 20.f, py + 15.f);
    statusMsg_.setFont(theme.regular); statusMsg_.setCharacterSize(theme.fontSizeSmall);
    statusMsg_.setFillColor(theme.errorText);
    statusMsg_.setPosition(px + 20.f, py + ph - 50.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 480.f }, theme);
    btnPay_ = Button({ px + 20.f, py + ph - 90.f, 180.f, theme.buttonHeight }, "Pay Selected", theme);
    btnBack_ = Button({ px + 220.f, py + ph - 90.f, 120.f, theme.buttonHeight }, "Back", theme);
}

PayBillPanel::~PayBillPanel() {}

void PayBillPanel::onEnter() { next_ = Screen::PayBill; statusMsg_.setString(""); loadUnpaidBills(); }

void PayBillPanel::loadUnpaidBills() {
    list_.clear(); billCount_ = 0;
    list_.addRow("BillID | ApptID | Amount (PKR) | Date", true);
    Patient* p = gui_->getLoggedInPatient(); if (!p) return;

    Bill* arr = gui_->getBills()->getAll();
    int n = gui_->getBills()->size();
    for (int i = 0; i < n; ++i) {
        if (arr[i].getPatientId() == p->getId() &&
            safeEqual(arr[i].getStatus(), "unpaid")) {
            char row[256]; row[0] = '\0';
            char bid[10], aid[10], ab[20];
            intToBuf(arr[i].getBillId(), bid);
            intToBuf(arr[i].getAppointmentId(), aid);
            floatToBuf(arr[i].getAmount(), ab);
            safeAppend(row, bid, 256); safeAppend(row, " | ", 256);
            safeAppend(row, aid, 256); safeAppend(row, " | PKR ", 256);
            safeAppend(row, ab, 256); safeAppend(row, " | ", 256);
            safeAppend(row, arr[i].getDate(), 256);
            list_.addRow(row);
            if (billCount_ < 100) billIds_[billCount_++] = arr[i].getBillId();
        }
    }
    if (billCount_ == 0) list_.addRow("No unpaid bills.");
}

void PayBillPanel::doPayment() {
    int sel = list_.getSelectedRow();
    int idx = sel - 1;
    if (sel < 1 || idx >= billCount_) { statusMsg_.setString("Select a bill."); return; }

    int billId = billIds_[idx];
    Bill* bill = gui_->getBills()->findById(billId);
    if (!bill || !safeEqual(bill->getStatus(), "unpaid")) { statusMsg_.setString("Invalid bill."); return; }

    Patient* p = gui_->getLoggedInPatient();
    if (p->getBalance() < bill->getAmount()) {
        statusMsg_.setString("Insufficient balance. Please top up.");
        return;
    }
    *p -= bill->getAmount();
    bill->setStatus("paid");
    FileHandler::updateBill(*gui_->getBills());
    FileHandler::updatePatient(*gui_->getPatients());

    char msg[80] = "Bill paid! Remaining balance: PKR ";
    char fb[20]; floatToBuf(p->getBalance(), fb);
    safeAppend(msg, fb, 80);
    gui_->showMessage(msg, Screen::PatientMenu, false);
}

void PayBillPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked;
    list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnPay_.update(mpos, mouseDown); btnBack_.update(mpos, mouseDown);
    if (btnPay_.isClicked())  doPayment();
    if (btnBack_.isClicked()) next_ = Screen::PatientMenu;
}

void PayBillPanel::update(float) {}
void PayBillPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); t.draw(statusMsg_);
    list_.draw(t); btnPay_.draw(t); btnBack_.draw(t);
}
Screen PayBillPanel::nextScreen() const { return next_; }

// ============================================================
//  TopUpPanel
// ============================================================
TopUpPanel::TopUpPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), attempts_(0), next_(Screen::TopUpBalance)
{
    float px = 400.f, py = 200.f, pw = 480.f, ph = 320.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Top Up Balance", theme, px + pw / 2.f, py + 20.f);
    {
        sf::FloatRect hb = heading_.getLocalBounds();
        heading_.setOrigin(hb.left + hb.width / 2.f, 0.f);
    }
    errorMsg_.setFont(theme.regular); errorMsg_.setCharacterSize(theme.fontSizeSmall);
    errorMsg_.setFillColor(theme.errorText);
    errorMsg_.setPosition(px + 20.f, py + 65.f);
    balanceInfo_.setFont(theme.regular); balanceInfo_.setCharacterSize(theme.fontSizeBody);
    balanceInfo_.setFillColor(theme.labelText);
    balanceInfo_.setPosition(px + 20.f, py + 82.f);

    float fy = py + 110.f;
    lblAmount_ = Label({ px + 20.f, fy }, "Amount to add (PKR):", theme);
    inputAmount_ = InputBox({ px + 20.f, fy + 24.f, pw - 40.f, theme.inputHeight }, "e.g. 5000", theme, 20);
    btnTopUp_ = Button({ px + 20.f, fy + 78.f, pw - 40.f, theme.buttonHeight }, "Top Up", theme);
    btnBack_ = Button({ px + 20.f, fy + 130.f, pw - 40.f, theme.buttonHeight }, "Back", theme);
}

TopUpPanel::~TopUpPanel() {}

void TopUpPanel::onEnter() {
    next_ = Screen::TopUpBalance;
    attempts_ = 0;
    errorMsg_.setString("");
    inputAmount_.clear();
    Patient* p = gui_->getLoggedInPatient();
    if (p) {
        char msg[60] = "Current balance: PKR ";
        char fb[20]; floatToBuf(p->getBalance(), fb);
        safeAppend(msg, fb, 60);
        balanceInfo_.setString(msg);
    }
}

void TopUpPanel::doTopUp() {
    const char* s = inputAmount_.getText();
    float amount = Utility::strToFloat(s);
    if (!Validator::validatePositiveFloat(amount)) {
        ++attempts_;
        if (attempts_ >= 3) {
            gui_->showMessage("Too many invalid attempts.", Screen::PatientMenu, true);
            return;
        }
        errorMsg_.setString("Amount must be positive (> 0). Please try again.");
        inputAmount_.clear();
        return;
    }
    Patient* p = gui_->getLoggedInPatient();
    *p += amount;
    FileHandler::updatePatient(*gui_->getPatients());

    char msg[80] = "Balance updated. New balance: PKR ";
    char fb[20]; floatToBuf(p->getBalance(), fb);
    safeAppend(msg, fb, 80);
    gui_->showMessage(msg, Screen::PatientMenu, false);
}

void TopUpPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked;
    inputAmount_.handleEvent(ev); inputAmount_.update(mpos, mouseDown);
    btnTopUp_.update(mpos, mouseDown); btnBack_.update(mpos, mouseDown);
    if (btnTopUp_.isClicked() ||
        (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Return))
        doTopUp();
    if (btnBack_.isClicked()) next_ = Screen::PatientMenu;
}

void TopUpPanel::update(float) {}
void TopUpPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); t.draw(errorMsg_); t.draw(balanceInfo_);
    lblAmount_.draw(t); inputAmount_.draw(t); btnTopUp_.draw(t); btnBack_.draw(t);
}
Screen TopUpPanel::nextScreen() const { return next_; }

// ============================================================
//  DoctorMenuPanel
// ============================================================
DoctorMenuPanel::DoctorMenuPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::DoctorMenu)
{
    float bw = 380.f, bh = theme.buttonHeight;
    float cx = 640.f, sy = 160.f, gap = 14.f;

    headerBar_.setSize({ 1280.f, 80.f });
    headerBar_.setPosition(0.f, 0.f);
    headerBar_.setFillColor(theme.headingText);

    lblWelcome_.setFont(theme.bold);
    lblWelcome_.setCharacterSize(20);
    lblWelcome_.setFillColor(sf::Color::White);
    lblWelcome_.setPosition(20.f, 28.f);

    auto mkBtn = [&](Button& b, const char* lbl, int i) {
        b = Button({ cx - bw / 2.f, sy + i * (bh + gap), bw, bh }, lbl, theme);
        };
    mkBtn(btnViewToday_, "1. View Today's Appointments", 0);
    mkBtn(btnMarkCompleted_, "2. Mark Appointment Complete", 1);
    mkBtn(btnMarkNoShow_, "3. Mark Appointment No-Show", 2);
    mkBtn(btnWriteRx_, "4. Write Prescription", 3);
    mkBtn(btnPatientHistory_, "5. View Patient Medical History", 4);
    mkBtn(btnLogout_, "Logout", 5);
}

DoctorMenuPanel::~DoctorMenuPanel() {}

void DoctorMenuPanel::onEnter() {
    next_ = Screen::DoctorMenu;
    Doctor* d = gui_->getLoggedInDoctor();
    if (d) {
        char msg[120] = "Welcome, Dr. ";
        safeAppend(msg, d->getName(), 120);
        safeAppend(msg, "  |  Specialization: ", 120);
        safeAppend(msg, d->getSpecialization(), 120);
        lblWelcome_.setString(msg);
    }
}

void DoctorMenuPanel::handleEvent(const sf::Event&, sf::Vector2f mpos,
    bool, bool mouseDown) {
    btnViewToday_.update(mpos, mouseDown);
    btnMarkCompleted_.update(mpos, mouseDown);
    btnMarkNoShow_.update(mpos, mouseDown);
    btnWriteRx_.update(mpos, mouseDown);
    btnPatientHistory_.update(mpos, mouseDown);
    btnLogout_.update(mpos, mouseDown);

    if (btnViewToday_.isClicked()) next_ = Screen::ViewToday;
    if (btnMarkCompleted_.isClicked()) next_ = Screen::MarkCompleted;
    if (btnMarkNoShow_.isClicked()) next_ = Screen::MarkNoShow;
    if (btnWriteRx_.isClicked()) next_ = Screen::WritePrescription;
    if (btnPatientHistory_.isClicked()) next_ = Screen::ViewPatientHistory;
    if (btnLogout_.isClicked()) { gui_->clearSession(); next_ = Screen::RoleSelect; }
}

void DoctorMenuPanel::update(float) {}
void DoctorMenuPanel::draw(sf::RenderTarget& t) const {
    t.draw(headerBar_); t.draw(lblWelcome_);
    btnViewToday_.draw(t); btnMarkCompleted_.draw(t); btnMarkNoShow_.draw(t);
    btnWriteRx_.draw(t); btnPatientHistory_.draw(t); btnLogout_.draw(t);
}
Screen DoctorMenuPanel::nextScreen() const { return next_; }

// ============================================================
//  ViewTodayPanel
// ============================================================
ViewTodayPanel::ViewTodayPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::ViewToday)
{
    float px = 80.f, py = 70.f, pw = 1120.f, ph = 680.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Today's Appointments", theme, px + 20.f, py + 15.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 560.f }, theme);
    btnBack_ = Button({ px + 20.f, py + ph - 55.f, 120.f, theme.buttonHeight }, "Back", theme);
}

ViewTodayPanel::~ViewTodayPanel() {}

void ViewTodayPanel::onEnter() { next_ = Screen::ViewToday; loadTodayAppointments(); }

void ViewTodayPanel::loadTodayAppointments() {
    list_.clear();
    char today[12]; Utility::getTodayDate(today);
    list_.addRow("ID   | Patient Name             | Slot  | Status", true);

    Doctor* doc = gui_->getLoggedInDoctor(); if (!doc) return;
    Appointment* arr = gui_->getAppointments()->getAll();
    int n = gui_->getAppointments()->size();
    Appointment* todays[100]; int cnt = 0;

    for (int i = 0; i < n && cnt < 100; ++i) {
        if (arr[i].getDoctorId() == doc->getId() &&
            safeEqual(arr[i].getDate(), today))
            todays[cnt++] = &arr[i];
    }
    // Sort by slot (lexicographic = time order)
    for (int i = 1; i < cnt; ++i) {
        Appointment* key = todays[i]; int j = i - 1;
        while (j >= 0 && Utility::strCmp(todays[j]->getTimeSlot(), key->getTimeSlot()) > 0) {
            todays[j + 1] = todays[j]; --j;
        }
        todays[j + 1] = key;
    }
    for (int i = 0; i < cnt; ++i) {
        Patient* p = gui_->getPatients()->findById(todays[i]->getPatientId());
        const char* pn = p ? p->getName() : "Unknown";
        char row[256]; row[0] = '\0';
        char idb[10]; intToBuf(todays[i]->getAppointmentId(), idb);
        safeAppend(row, idb, 256); safeAppend(row, " | ", 256);
        safeAppend(row, pn, 256); safeAppend(row, " | ", 256);
        safeAppend(row, todays[i]->getTimeSlot(), 256); safeAppend(row, " | ", 256);
        safeAppend(row, todays[i]->getStatus(), 256);
        list_.addRow(row);
    }
    if (cnt == 0) list_.addRow("No appointments scheduled for today.");
}

void ViewTodayPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked; list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnBack_.isClicked()) next_ = Screen::DoctorMenu;
}
void ViewTodayPanel::update(float) {}
void ViewTodayPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); list_.draw(t); btnBack_.draw(t);
}
Screen ViewTodayPanel::nextScreen() const { return next_; }

// ============================================================
//  MarkCompletedPanel
// ============================================================
MarkCompletedPanel::MarkCompletedPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), apptCount_(0), next_(Screen::MarkCompleted)
{
    float px = 120.f, py = 80.f, pw = 1040.f, ph = 660.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Mark Appointment Complete", theme, px + 20.f, py + 15.f);
    statusMsg_.setFont(theme.regular); statusMsg_.setCharacterSize(theme.fontSizeSmall);
    statusMsg_.setFillColor(theme.successText);
    statusMsg_.setPosition(px + 20.f, py + ph - 50.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 480.f }, theme);
    btnMark_ = Button({ px + 20.f, py + ph - 90.f, 200.f, theme.buttonHeight }, "Mark Complete", theme);
    btnBack_ = Button({ px + 240.f, py + ph - 90.f, 120.f, theme.buttonHeight }, "Back", theme);
}

MarkCompletedPanel::~MarkCompletedPanel() {}

void MarkCompletedPanel::onEnter() {
    next_ = Screen::MarkCompleted;
    statusMsg_.setString("");
    loadPendingTodayAppointments();
}

void MarkCompletedPanel::loadPendingTodayAppointments() {
    list_.clear(); apptCount_ = 0;
    char today[12]; Utility::getTodayDate(today);
    list_.addRow("ID   | Patient Name             | Slot  ", true);
    Doctor* doc = gui_->getLoggedInDoctor(); if (!doc) return;
    Appointment* arr = gui_->getAppointments()->getAll();
    int n = gui_->getAppointments()->size();
    for (int i = 0; i < n; ++i) {
        if (arr[i].getDoctorId() == doc->getId() &&
            safeEqual(arr[i].getDate(), today) &&
            safeEqual(arr[i].getStatus(), "pending")) {
            Patient* p = gui_->getPatients()->findById(arr[i].getPatientId());
            const char* pn = p ? p->getName() : "Unknown";
            char row[256]; row[0] = '\0';
            char idb[10]; intToBuf(arr[i].getAppointmentId(), idb);
            safeAppend(row, idb, 256); safeAppend(row, " | ", 256);
            safeAppend(row, pn, 256); safeAppend(row, " | ", 256);
            safeAppend(row, arr[i].getTimeSlot(), 256);
            list_.addRow(row);
            if (apptCount_ < 100) apptIds_[apptCount_++] = arr[i].getAppointmentId();
        }
    }
    if (apptCount_ == 0) list_.addRow("No pending appointments for today.");
}

void MarkCompletedPanel::doMarkCompleted() {
    int sel = list_.getSelectedRow();
    int idx = sel - 1;
    if (sel < 1 || idx >= apptCount_) { statusMsg_.setString("Select an appointment."); return; }

    Appointment* appt = gui_->getAppointments()->findById(apptIds_[idx]);
    if (!appt) { statusMsg_.setString("Appointment not found."); return; }
    appt->setStatus("completed");
    FileHandler::updateAppointment(*gui_->getAppointments());
    gui_->showMessage("Appointment marked as completed.", Screen::DoctorMenu, false);
}

void MarkCompletedPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked; list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnMark_.update(mpos, mouseDown); btnBack_.update(mpos, mouseDown);
    if (btnMark_.isClicked()) doMarkCompleted();
    if (btnBack_.isClicked()) next_ = Screen::DoctorMenu;
}
void MarkCompletedPanel::update(float) {}
void MarkCompletedPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); t.draw(statusMsg_);
    list_.draw(t); btnMark_.draw(t); btnBack_.draw(t);
}
Screen MarkCompletedPanel::nextScreen() const { return next_; }

// ============================================================
//  MarkNoShowPanel
// ============================================================
MarkNoShowPanel::MarkNoShowPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), apptCount_(0), next_(Screen::MarkNoShow)
{
    float px = 120.f, py = 80.f, pw = 1040.f, ph = 660.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Mark Appointment No-Show", theme, px + 20.f, py + 15.f);
    statusMsg_.setFont(theme.regular); statusMsg_.setCharacterSize(theme.fontSizeSmall);
    statusMsg_.setFillColor(theme.errorText);
    statusMsg_.setPosition(px + 20.f, py + ph - 50.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 480.f }, theme);
    btnMark_ = Button({ px + 20.f, py + ph - 90.f, 200.f, theme.buttonHeight }, "Mark No-Show", theme);
    btnBack_ = Button({ px + 240.f, py + ph - 90.f, 120.f, theme.buttonHeight }, "Back", theme);
}

MarkNoShowPanel::~MarkNoShowPanel() {}

void MarkNoShowPanel::onEnter() {
    next_ = Screen::MarkNoShow;
    statusMsg_.setString("");
    loadPendingTodayAppointments();
}

void MarkNoShowPanel::loadPendingTodayAppointments() {
    list_.clear(); apptCount_ = 0;
    char today[12]; Utility::getTodayDate(today);
    list_.addRow("ID   | Patient Name             | Slot  ", true);
    Doctor* doc = gui_->getLoggedInDoctor(); if (!doc) return;
    Appointment* arr = gui_->getAppointments()->getAll();
    int n = gui_->getAppointments()->size();
    for (int i = 0; i < n; ++i) {
        if (arr[i].getDoctorId() == doc->getId() &&
            safeEqual(arr[i].getDate(), today) &&
            safeEqual(arr[i].getStatus(), "pending")) {
            Patient* p = gui_->getPatients()->findById(arr[i].getPatientId());
            const char* pn = p ? p->getName() : "Unknown";
            char row[256]; row[0] = '\0';
            char idb[10]; intToBuf(arr[i].getAppointmentId(), idb);
            safeAppend(row, idb, 256); safeAppend(row, " | ", 256);
            safeAppend(row, pn, 256); safeAppend(row, " | ", 256);
            safeAppend(row, arr[i].getTimeSlot(), 256);
            list_.addRow(row);
            if (apptCount_ < 100) apptIds_[apptCount_++] = arr[i].getAppointmentId();
        }
    }
    if (apptCount_ == 0) list_.addRow("No pending appointments for today.");
}

void MarkNoShowPanel::doMarkNoShow() {
    int sel = list_.getSelectedRow();
    int idx = sel - 1;
    if (sel < 1 || idx >= apptCount_) { statusMsg_.setString("Select an appointment."); return; }

    int apptId = apptIds_[idx];
    Appointment* appt = gui_->getAppointments()->findById(apptId);
    if (!appt) { statusMsg_.setString("Appointment not found."); return; }
    appt->setStatus("noshow");
    FileHandler::updateAppointment(*gui_->getAppointments());

    // Cancel the corresponding bill (no refund)
    Bill* bills = gui_->getBills()->getAll();
    int bn = gui_->getBills()->size();
    for (int i = 0; i < bn; ++i) {
        if (bills[i].getAppointmentId() == apptId) {
            bills[i].setStatus("cancelled");
            FileHandler::updateBill(*gui_->getBills());
            break;
        }
    }
    gui_->showMessage("Appointment marked as no-show.", Screen::DoctorMenu, false);
}

void MarkNoShowPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked; list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnMark_.update(mpos, mouseDown); btnBack_.update(mpos, mouseDown);
    if (btnMark_.isClicked()) doMarkNoShow();
    if (btnBack_.isClicked()) next_ = Screen::DoctorMenu;
}
void MarkNoShowPanel::update(float) {}
void MarkNoShowPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); t.draw(statusMsg_);
    list_.draw(t); btnMark_.draw(t); btnBack_.draw(t);
}
Screen MarkNoShowPanel::nextScreen() const { return next_; }

// ============================================================
//  WritePrescriptionPanel
// ============================================================
WritePrescriptionPanel::WritePrescriptionPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), step_(0), selectedApptId_(-1),
    next_(Screen::WritePrescription)
{
    float px = 120.f, py = 70.f, pw = 1040.f, ph = 700.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Write Prescription", theme, px + 20.f, py + 15.f);
    statusMsg_.setFont(theme.regular); statusMsg_.setCharacterSize(theme.fontSizeSmall);
    statusMsg_.setFillColor(theme.errorText);
    statusMsg_.setPosition(px + 20.f, py + 65.f);

    float fy = py + 90.f;
    lblApptId_ = Label({ px + 20.f, fy }, "Completed Appointment ID:", theme);
    inputApptId_ = InputBox({ px + 20.f, fy + 24.f, 200.f, theme.inputHeight }, "Appt ID", theme, 12);
    btnFindAppt_ = Button({ px + 240.f, fy + 22.f, 120.f, theme.buttonHeight + 2.f }, "Find", theme);

    lblMedicines_ = Label({ px + 20.f, fy + 80.f }, "Medicines (semicolon-separated):", theme);
    inputMedicines_ = InputBox({ px + 20.f, fy + 104.f, pw - 40.f, theme.inputHeight }, "e.g. Paracetamol 500mg;Amoxicillin 250mg", theme, 499);
    lblNotes_ = Label({ px + 20.f, fy + 156.f }, "Notes (max 300 chars):", theme);
    inputNotes_ = InputBox({ px + 20.f, fy + 180.f, pw - 40.f, theme.inputHeight }, "Doctor notes...", theme, 299);
    btnSaveRx_ = Button({ px + 20.f, fy + 234.f, 160.f, theme.buttonHeight }, "Save Prescription", theme);
    btnBack_ = Button({ px + 200.f, fy + 234.f, 120.f, theme.buttonHeight }, "Back", theme);
}

WritePrescriptionPanel::~WritePrescriptionPanel() {}

void WritePrescriptionPanel::onEnter() {
    next_ = Screen::WritePrescription;
    step_ = 0; selectedApptId_ = -1;
    statusMsg_.setString("");
    inputApptId_.clear(); inputMedicines_.clear(); inputNotes_.clear();
}

void WritePrescriptionPanel::doFindAppointment() {
    int id = Validator::charToInt(inputApptId_.getText());
    Doctor* doc = gui_->getLoggedInDoctor();
    Appointment* appt = gui_->getAppointments()->findById(id);

    if (!appt || appt->getDoctorId() != doc->getId() ||
        !safeEqual(appt->getStatus(), "completed")) {
        statusMsg_.setString("Appointment not found or not completed by you.");
        return;
    }
    // Check for duplicate
    Prescription* prArr = gui_->getPrescriptions()->getAll();
    int prN = gui_->getPrescriptions()->size();
    for (int i = 0; i < prN; ++i) {
        if (prArr[i].getAppointmentId() == id) {
            statusMsg_.setString("Prescription already written for this appointment.");
            return;
        }
    }
    selectedApptId_ = id;
    step_ = 1;
    statusMsg_.setString("");
}

void WritePrescriptionPanel::doSavePrescription() {
    const char* med = inputMedicines_.getText();
    const char* notes = inputNotes_.getText();
    if (Validator::strLen(med) == 0) {
        statusMsg_.setString("Please enter medicines.");
        return;
    }
    Doctor* doc = gui_->getLoggedInDoctor();
    Appointment* appt = gui_->getAppointments()->findById(selectedApptId_);

    int newId = gui_->getPrescriptions()->getMaxId() + 1;
    Prescription pr(newId, selectedApptId_, appt->getPatientId(),
        doc->getId(), appt->getDate(), med, notes);
    gui_->getPrescriptions()->add(pr);
    FileHandler::appendPrescription(pr);
    gui_->showMessage("Prescription saved.", Screen::DoctorMenu, false);
}

void WritePrescriptionPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked;
    if (step_ == 0) {
        inputApptId_.handleEvent(ev); inputApptId_.update(mpos, mouseDown);
        btnFindAppt_.update(mpos, mouseDown);
        if (btnFindAppt_.isClicked() ||
            (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Return))
            doFindAppointment();
    }
    else {
        inputMedicines_.handleEvent(ev); inputMedicines_.update(mpos, mouseDown);
        inputNotes_.handleEvent(ev); inputNotes_.update(mpos, mouseDown);
        btnSaveRx_.update(mpos, mouseDown);
        if (btnSaveRx_.isClicked()) doSavePrescription();
    }
    btnBack_.update(mpos, mouseDown);
    if (btnBack_.isClicked()) next_ = Screen::DoctorMenu;
}
void WritePrescriptionPanel::update(float) {}
void WritePrescriptionPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); t.draw(statusMsg_);
    if (step_ == 0) {
        lblApptId_.draw(t); inputApptId_.draw(t); btnFindAppt_.draw(t);
    }
    else {
        lblMedicines_.draw(t); inputMedicines_.draw(t);
        lblNotes_.draw(t);     inputNotes_.draw(t);
        btnSaveRx_.draw(t);
    }
    btnBack_.draw(t);
}
Screen WritePrescriptionPanel::nextScreen() const { return next_; }

// ============================================================
//  ViewPatientHistoryPanel
// ============================================================
ViewPatientHistoryPanel::ViewPatientHistoryPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::ViewPatientHistory)
{
    float px = 100.f, py = 70.f, pw = 1080.f, ph = 700.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Patient Medical History", theme, px + 20.f, py + 15.f);
    statusMsg_.setFont(theme.regular); statusMsg_.setCharacterSize(theme.fontSizeSmall);
    statusMsg_.setFillColor(theme.errorText);
    statusMsg_.setPosition(px + 20.f, py + 65.f);

    float fy = py + 90.f;
    lblPatientId_ = Label({ px + 20.f, fy }, "Patient ID:", theme);
    inputPatientId_ = InputBox({ px + 20.f, fy + 24.f, 200.f, theme.inputHeight }, "Patient ID", theme, 12);
    btnSearch_ = Button({ px + 240.f, fy + 22.f, 120.f, theme.buttonHeight + 2.f }, "Search", theme);
    list_ = ScrollableList({ px + 20.f, fy + 80.f, pw - 40.f, 460.f }, theme);
    btnBack_ = Button({ px + 20.f, py + ph - 55.f, 120.f, theme.buttonHeight }, "Back", theme);
}

ViewPatientHistoryPanel::~ViewPatientHistoryPanel() {}
void ViewPatientHistoryPanel::onEnter() {
    next_ = Screen::ViewPatientHistory;
    statusMsg_.setString(""); list_.clear(); inputPatientId_.clear();
}

void ViewPatientHistoryPanel::doSearch() {
    int patId = Validator::charToInt(inputPatientId_.getText());
    Doctor* doc = gui_->getLoggedInDoctor();
    Patient* pat = gui_->getPatients()->findById(patId);

    if (!pat) {
        statusMsg_.setString("Access denied. You can only view records of your own patients.");
        list_.clear(); return;
    }
    // Check: must have at least one completed appt with this doctor
    Appointment* arr = gui_->getAppointments()->getAll();
    int n = gui_->getAppointments()->size();
    bool isMyPatient = false;
    for (int i = 0; i < n; ++i) {
        if (arr[i].getDoctorId() == doc->getId() &&
            arr[i].getPatientId() == patId &&
            safeEqual(arr[i].getStatus(), "completed")) {
            isMyPatient = true; break;
        }
    }
    if (!isMyPatient) {
        statusMsg_.setString("Access denied. You can only view records of your own patients.");
        list_.clear(); return;
    }
    statusMsg_.setString("");
    list_.clear();

    Prescription* prArr = gui_->getPrescriptions()->getAll();
    int prN = gui_->getPrescriptions()->size();
    Prescription* myPr[100]; int cnt = 0;
    for (int i = 0; i < prN && cnt < 100; ++i)
        if (prArr[i].getDoctorId() == doc->getId() && prArr[i].getPatientId() == patId)
            myPr[cnt++] = &prArr[i];

    if (cnt == 0) { list_.addRow("No prescriptions found for this patient."); return; }
    Utility::sortPrescriptionsDesc(myPr, cnt);

    char hdr[100] = "Patient: "; safeAppend(hdr, pat->getName(), 100);
    list_.addRow(hdr, true);
    for (int i = 0; i < cnt; ++i) {
        char drow[100] = "Date: "; safeAppend(drow, myPr[i]->getDate(), 100);
        list_.addRow(drow, true);
        char mrow[512] = "Medicines: "; safeAppend(mrow, myPr[i]->getMedicines(), 512);
        list_.addRow(mrow);
        char nrow[350] = "Notes: "; safeAppend(nrow, myPr[i]->getNotes(), 350);
        list_.addRow(nrow);
    }
}

void ViewPatientHistoryPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked;
    inputPatientId_.handleEvent(ev); inputPatientId_.update(mpos, mouseDown);
    list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnSearch_.update(mpos, mouseDown); btnBack_.update(mpos, mouseDown);
    if (btnSearch_.isClicked() ||
        (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Return))
        doSearch();
    if (btnBack_.isClicked()) next_ = Screen::DoctorMenu;
}
void ViewPatientHistoryPanel::update(float) {}
void ViewPatientHistoryPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); t.draw(statusMsg_);
    lblPatientId_.draw(t); inputPatientId_.draw(t); btnSearch_.draw(t);
    list_.draw(t); btnBack_.draw(t);
}
Screen ViewPatientHistoryPanel::nextScreen() const { return next_; }

// ============================================================
//  AdminMenuPanel
// ============================================================
AdminMenuPanel::AdminMenuPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::AdminMenu)
{
    float bw = 360.f, bh = theme.buttonHeight;
    float cx = 640.f, sy = 120.f, gap = 10.f;

    headerBar_.setSize({ 1280.f, 70.f });
    headerBar_.setPosition(0.f, 0.f);
    headerBar_.setFillColor(theme.headingText);

    lblHeading_.setFont(theme.bold);
    lblHeading_.setCharacterSize(20);
    lblHeading_.setFillColor(sf::Color::White);
    lblHeading_.setString("Admin Panel -- MediCore");
    lblHeading_.setPosition(20.f, 22.f);

    auto mkBtn = [&](Button& b, const char* lbl, int row, int col) {
        float x = (col == 0) ? cx - bw - 10.f : cx + 10.f;
        b = Button({ x, sy + row * (bh + gap), bw, bh }, lbl, theme);
        };
    mkBtn(btnAddDoctor_, "1.  Add Doctor", 0, 0);
    mkBtn(btnRemoveDoctor_, "2.  Remove Doctor", 0, 1);
    mkBtn(btnAllPatients_, "3.  View All Patients", 1, 0);
    mkBtn(btnAllDoctors_, "4.  View All Doctors", 1, 1);
    mkBtn(btnAllAppts_, "5.  View All Appointments", 2, 0);
    mkBtn(btnUnpaidBills_, "6.  View Unpaid Bills", 2, 1);
    mkBtn(btnDischarge_, "7.  Discharge Patient", 3, 0);
    mkBtn(btnSecLog_, "8.  View Security Log", 3, 1);
    mkBtn(btnReport_, "9.  Generate Daily Report", 4, 0);
    btnLogout_ = Button({ cx - bw / 2.f, sy + 5 * (bh + gap), bw, bh }, "Logout", theme);
}

AdminMenuPanel::~AdminMenuPanel() {}
void AdminMenuPanel::onEnter() { next_ = Screen::AdminMenu; }

void AdminMenuPanel::handleEvent(const sf::Event&, sf::Vector2f mpos,
    bool, bool mouseDown) {
    btnAddDoctor_.update(mpos, mouseDown); btnRemoveDoctor_.update(mpos, mouseDown);
    btnAllPatients_.update(mpos, mouseDown); btnAllDoctors_.update(mpos, mouseDown);
    btnAllAppts_.update(mpos, mouseDown); btnUnpaidBills_.update(mpos, mouseDown);
    btnDischarge_.update(mpos, mouseDown); btnSecLog_.update(mpos, mouseDown);
    btnReport_.update(mpos, mouseDown); btnLogout_.update(mpos, mouseDown);

    if (btnAddDoctor_.isClicked()) next_ = Screen::AddDoctor;
    if (btnRemoveDoctor_.isClicked()) next_ = Screen::RemoveDoctor;
    if (btnAllPatients_.isClicked()) next_ = Screen::ViewAllPatients;
    if (btnAllDoctors_.isClicked()) next_ = Screen::ViewAllDoctors;
    if (btnAllAppts_.isClicked()) next_ = Screen::ViewAllAppointments;
    if (btnUnpaidBills_.isClicked()) next_ = Screen::ViewUnpaidBills;
    if (btnDischarge_.isClicked()) next_ = Screen::DischargePatient;
    if (btnSecLog_.isClicked()) next_ = Screen::SecurityLog;
    if (btnReport_.isClicked()) next_ = Screen::DailyReport;
    if (btnLogout_.isClicked()) { gui_->clearSession(); next_ = Screen::RoleSelect; }
}
void AdminMenuPanel::update(float) {}
void AdminMenuPanel::draw(sf::RenderTarget& t) const {
    t.draw(headerBar_); t.draw(lblHeading_);
    btnAddDoctor_.draw(t);   btnRemoveDoctor_.draw(t);
    btnAllPatients_.draw(t); btnAllDoctors_.draw(t);
    btnAllAppts_.draw(t);    btnUnpaidBills_.draw(t);
    btnDischarge_.draw(t);   btnSecLog_.draw(t);
    btnReport_.draw(t);      btnLogout_.draw(t);
}
Screen AdminMenuPanel::nextScreen() const { return next_; }

// ============================================================
//  AddDoctorPanel
// ============================================================
AddDoctorPanel::AddDoctorPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::AddDoctor)
{
    float px = 200.f, py = 60.f, pw = 880.f, ph = 700.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Add Doctor", theme, px + pw / 2.f, py + 20.f);
    {
        sf::FloatRect hb = heading_.getLocalBounds();
        heading_.setOrigin(hb.left + hb.width / 2.f, 0.f);
    }
    errorMsg_.setFont(theme.regular); errorMsg_.setCharacterSize(theme.fontSizeSmall);
    errorMsg_.setFillColor(theme.errorText);
    errorMsg_.setPosition(px + 20.f, py + 65.f);

    float fy = py + 85.f, lh = 58.f, fw = pw - 40.f;
    lblName_ = Label({ px + 20.f, fy }, "Name (max 50):", theme);
    inputName_ = InputBox({ px + 20.f, fy + 20.f, fw, theme.inputHeight }, "", theme, 51);
    lblSpec_ = Label({ px + 20.f, fy + lh }, "Specialization:", theme);
    inputSpec_ = InputBox({ px + 20.f, fy + lh + 20.f, fw, theme.inputHeight }, "", theme, 51);
    lblContact_ = Label({ px + 20.f, fy + 2 * lh }, "Contact (11 digits):", theme);
    inputContact_ = InputBox({ px + 20.f, fy + 2 * lh + 20.f, fw, theme.inputHeight }, "", theme, 12);
    lblPassword_ = Label({ px + 20.f, fy + 3 * lh }, "Password (min 6):", theme);
    inputPassword_ = InputBox({ px + 20.f, fy + 3 * lh + 20.f, fw, theme.inputHeight }, "", theme, 51, true);
    lblFee_ = Label({ px + 20.f, fy + 4 * lh }, "Consultation Fee (PKR):", theme);
    inputFee_ = InputBox({ px + 20.f, fy + 4 * lh + 20.f, fw, theme.inputHeight }, "e.g. 1500", theme, 20);

    btnAdd_ = Button({ px + 20.f, fy + 5 * lh + 10.f, 160.f, theme.buttonHeight }, "Add Doctor", theme);
    btnBack_ = Button({ px + 200.f, fy + 5 * lh + 10.f, 120.f, theme.buttonHeight }, "Back", theme);
}

AddDoctorPanel::~AddDoctorPanel() {}
void AddDoctorPanel::onEnter() {
    next_ = Screen::AddDoctor;
    errorMsg_.setString("");
    inputName_.clear(); inputSpec_.clear(); inputContact_.clear();
    inputPassword_.clear(); inputFee_.clear();
}

void AddDoctorPanel::doAddDoctor() {
    const char* name = inputName_.getText();
    const char* spec = inputSpec_.getText();
    const char* cont = inputContact_.getText();
    const char* pass = inputPassword_.getText();
    float fee = Utility::strToFloat(inputFee_.getText());

    if (Validator::strLen(name) == 0) { errorMsg_.setString("Name cannot be empty."); return; }
    if (!Validator::validateContact(cont)) { errorMsg_.setString("Contact must be exactly 11 numeric digits."); return; }
    if (!Validator::validatePassword(pass)) { errorMsg_.setString("Password must be at least 6 characters."); return; }
    if (!Validator::validatePositiveFloat(fee)) { errorMsg_.setString("Fee must be a positive number."); return; }

    int newId = gui_->getDoctors()->getMaxId() + 1;
    Doctor d(newId, name, spec, cont, pass, fee);
    gui_->getDoctors()->add(d);
    FileHandler::appendDoctor(d);

    char msg[60] = "Doctor added. ID: ";
    char idb[12]; intToBuf(newId, idb);
    safeAppend(msg, idb, 60);
    gui_->showMessage(msg, Screen::AdminMenu, false);
}

void AddDoctorPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked;
    inputName_.handleEvent(ev); inputName_.update(mpos, mouseDown);
    inputSpec_.handleEvent(ev); inputSpec_.update(mpos, mouseDown);
    inputContact_.handleEvent(ev); inputContact_.update(mpos, mouseDown);
    inputPassword_.handleEvent(ev); inputPassword_.update(mpos, mouseDown);
    inputFee_.handleEvent(ev); inputFee_.update(mpos, mouseDown);
    btnAdd_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnAdd_.isClicked()) doAddDoctor();
    if (btnBack_.isClicked()) next_ = Screen::AdminMenu;
}
void AddDoctorPanel::update(float) {}
void AddDoctorPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); t.draw(errorMsg_);
    lblName_.draw(t);     inputName_.draw(t);
    lblSpec_.draw(t);     inputSpec_.draw(t);
    lblContact_.draw(t);  inputContact_.draw(t);
    lblPassword_.draw(t); inputPassword_.draw(t);
    lblFee_.draw(t);      inputFee_.draw(t);
    btnAdd_.draw(t); btnBack_.draw(t);
}
Screen AddDoctorPanel::nextScreen() const { return next_; }

// ============================================================
//  RemoveDoctorPanel
// ============================================================
RemoveDoctorPanel::RemoveDoctorPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), doctorCount_(0), next_(Screen::RemoveDoctor)
{
    float px = 120.f, py = 80.f, pw = 1040.f, ph = 660.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Remove Doctor", theme, px + 20.f, py + 15.f);
    statusMsg_.setFont(theme.regular); statusMsg_.setCharacterSize(theme.fontSizeSmall);
    statusMsg_.setFillColor(theme.errorText);
    statusMsg_.setPosition(px + 20.f, py + ph - 50.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 480.f }, theme);
    btnRemove_ = Button({ px + 20.f, py + ph - 90.f, 180.f, theme.buttonHeight }, "Remove Selected", theme);
    btnBack_ = Button({ px + 220.f, py + ph - 90.f, 120.f, theme.buttonHeight }, "Back", theme);
}

RemoveDoctorPanel::~RemoveDoctorPanel() {}
void RemoveDoctorPanel::onEnter() { next_ = Screen::RemoveDoctor; statusMsg_.setString(""); loadDoctors(); }

void RemoveDoctorPanel::loadDoctors() {
    list_.clear(); doctorCount_ = 0;
    list_.addRow("ID  | Name                     | Specialization       | Fee", true);
    Doctor* arr = gui_->getDoctors()->getAll();
    int n = gui_->getDoctors()->size();
    for (int i = 0; i < n; ++i) {
        char row[256]; row[0] = '\0';
        char idb[10], fb[20];
        intToBuf(arr[i].getId(), idb); floatToBuf(arr[i].getFee(), fb);
        safeAppend(row, idb, 256); safeAppend(row, " | ", 256);
        safeAppend(row, arr[i].getName(), 256); safeAppend(row, " | ", 256);
        safeAppend(row, arr[i].getSpecialization(), 256); safeAppend(row, " | PKR ", 256);
        safeAppend(row, fb, 256);
        list_.addRow(row);
        if (doctorCount_ < 100) doctorIds_[doctorCount_++] = arr[i].getId();
    }
    if (doctorCount_ == 0) list_.addRow("No doctors registered.");
}

void RemoveDoctorPanel::doRemove() {
    int sel = list_.getSelectedRow();
    int idx = sel - 1;
    if (sel < 1 || idx >= doctorCount_) { statusMsg_.setString("Select a doctor."); return; }

    int docId = doctorIds_[idx];
    Appointment* arr = gui_->getAppointments()->getAll();
    int n = gui_->getAppointments()->size();
    for (int i = 0; i < n; ++i) {
        if (arr[i].getDoctorId() == docId && safeEqual(arr[i].getStatus(), "pending")) {
            statusMsg_.setString("Cannot remove doctor with pending appointments.");
            return;
        }
    }
    FileHandler::deleteDoctor(docId, *gui_->getDoctors());
    gui_->showMessage("Doctor removed.", Screen::AdminMenu, false);
}

void RemoveDoctorPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked; list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnRemove_.update(mpos, mouseDown); btnBack_.update(mpos, mouseDown);
    if (btnRemove_.isClicked()) doRemove();
    if (btnBack_.isClicked()) next_ = Screen::AdminMenu;
}
void RemoveDoctorPanel::update(float) {}
void RemoveDoctorPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); t.draw(statusMsg_);
    list_.draw(t); btnRemove_.draw(t); btnBack_.draw(t);
}
Screen RemoveDoctorPanel::nextScreen() const { return next_; }

// ============================================================
//  ViewAllPatientsPanel
// ============================================================
ViewAllPatientsPanel::ViewAllPatientsPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::ViewAllPatients)
{
    float px = 40.f, py = 70.f, pw = 1200.f, ph = 680.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("All Patients", theme, px + 20.f, py + 15.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 560.f }, theme);
    btnBack_ = Button({ px + 20.f, py + ph - 55.f, 120.f, theme.buttonHeight }, "Back", theme);
}

ViewAllPatientsPanel::~ViewAllPatientsPanel() {}
void ViewAllPatientsPanel::onEnter() { next_ = Screen::ViewAllPatients; loadPatients(); }

void ViewAllPatientsPanel::loadPatients() {
    list_.clear();
    list_.addRow("ID | Name                | Age | G | Contact       | Balance     | Unpaid Bills", true);
    Patient* arr = gui_->getPatients()->getAll();
    int n = gui_->getPatients()->size();
    Bill* bills = gui_->getBills()->getAll();
    int bn = gui_->getBills()->size();

    for (int i = 0; i < n; ++i) {
        int unpaid = 0;
        for (int j = 0; j < bn; ++j)
            if (bills[j].getPatientId() == arr[i].getId() && safeEqual(bills[j].getStatus(), "unpaid"))
                ++unpaid;

        char row[300]; row[0] = '\0';
        char idb[10], ageb[8], bb[20], ub[8];
        intToBuf(arr[i].getId(), idb); intToBuf(arr[i].getAge(), ageb);
        floatToBuf(arr[i].getBalance(), bb); intToBuf(unpaid, ub);
        char gb[3] = { arr[i].getGender(), '\0' };

        safeAppend(row, idb, 300); safeAppend(row, " | ", 300);
        safeAppend(row, arr[i].getName(), 300); safeAppend(row, " | ", 300);
        safeAppend(row, ageb, 300); safeAppend(row, " | ", 300);
        safeAppend(row, gb, 300); safeAppend(row, " | ", 300);
        safeAppend(row, arr[i].getContact(), 300); safeAppend(row, " | PKR ", 300);
        safeAppend(row, bb, 300); safeAppend(row, " | ", 300);
        safeAppend(row, ub, 300);
        list_.addRow(row);
    }
    if (n == 0) list_.addRow("No patients registered.");
}

void ViewAllPatientsPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked; list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnBack_.isClicked()) next_ = Screen::AdminMenu;
}
void ViewAllPatientsPanel::update(float) {}
void ViewAllPatientsPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); list_.draw(t); btnBack_.draw(t);
}
Screen ViewAllPatientsPanel::nextScreen() const { return next_; }

// ============================================================
//  ViewAllDoctorsPanel
// ============================================================
ViewAllDoctorsPanel::ViewAllDoctorsPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::ViewAllDoctors)
{
    float px = 60.f, py = 70.f, pw = 1160.f, ph = 680.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("All Doctors", theme, px + 20.f, py + 15.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 560.f }, theme);
    btnBack_ = Button({ px + 20.f, py + ph - 55.f, 120.f, theme.buttonHeight }, "Back", theme);
}

ViewAllDoctorsPanel::~ViewAllDoctorsPanel() {}
void ViewAllDoctorsPanel::onEnter() { next_ = Screen::ViewAllDoctors; loadDoctors(); }

void ViewAllDoctorsPanel::loadDoctors() {
    list_.clear();
    list_.addRow("ID | Name                 | Specialization        | Contact     | Fee", true);
    Doctor* arr = gui_->getDoctors()->getAll();
    int n = gui_->getDoctors()->size();
    for (int i = 0; i < n; ++i) {
        char row[256]; row[0] = '\0';
        char idb[10], fb[20];
        intToBuf(arr[i].getId(), idb); floatToBuf(arr[i].getFee(), fb);
        safeAppend(row, idb, 256); safeAppend(row, " | ", 256);
        safeAppend(row, arr[i].getName(), 256); safeAppend(row, " | ", 256);
        safeAppend(row, arr[i].getSpecialization(), 256); safeAppend(row, " | ", 256);
        safeAppend(row, arr[i].getContact(), 256); safeAppend(row, " | PKR ", 256);
        safeAppend(row, fb, 256);
        list_.addRow(row);
    }
    if (n == 0) list_.addRow("No doctors registered.");
}

void ViewAllDoctorsPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked; list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnBack_.isClicked()) next_ = Screen::AdminMenu;
}
void ViewAllDoctorsPanel::update(float) {}
void ViewAllDoctorsPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); list_.draw(t); btnBack_.draw(t);
}
Screen ViewAllDoctorsPanel::nextScreen() const { return next_; }

// ============================================================
//  ViewAllAppointmentsPanel
// ============================================================
ViewAllAppointmentsPanel::ViewAllAppointmentsPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::ViewAllAppointments)
{
    float px = 40.f, py = 70.f, pw = 1200.f, ph = 680.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("All Appointments", theme, px + 20.f, py + 15.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 560.f }, theme);
    btnBack_ = Button({ px + 20.f, py + ph - 55.f, 120.f, theme.buttonHeight }, "Back", theme);
}

ViewAllAppointmentsPanel::~ViewAllAppointmentsPanel() {}
void ViewAllAppointmentsPanel::onEnter() { next_ = Screen::ViewAllAppointments; loadAppointments(); }

void ViewAllAppointmentsPanel::loadAppointments() {
    list_.clear();
    list_.addRow("ID  | Patient              | Doctor               | Date       | Slot  | Status", true);

    Appointment* arr = gui_->getAppointments()->getAll();
    int n = gui_->getAppointments()->size();
    Appointment* sorted[100]; int cnt = n < 100 ? n : 100;
    for (int i = 0; i < cnt; ++i) sorted[i] = &arr[i];
    Utility::sortAppointmentsDesc(sorted, cnt);

    for (int i = 0; i < cnt; ++i) {
        Patient* p = gui_->getPatients()->findById(sorted[i]->getPatientId());
        Doctor* d = gui_->getDoctors()->findById(sorted[i]->getDoctorId());
        const char* pn = p ? p->getName() : "Unknown";
        const char* dn = d ? d->getName() : "Unknown";
        char row[300]; row[0] = '\0';
        char idb[10]; intToBuf(sorted[i]->getAppointmentId(), idb);
        safeAppend(row, idb, 300); safeAppend(row, " | ", 300);
        safeAppend(row, pn, 300); safeAppend(row, " | ", 300);
        safeAppend(row, dn, 300); safeAppend(row, " | ", 300);
        safeAppend(row, sorted[i]->getDate(), 300); safeAppend(row, " | ", 300);
        safeAppend(row, sorted[i]->getTimeSlot(), 300); safeAppend(row, " | ", 300);
        safeAppend(row, sorted[i]->getStatus(), 300);
        list_.addRow(row);
    }
    if (cnt == 0) list_.addRow("No appointments found.");
}

void ViewAllAppointmentsPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked; list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnBack_.isClicked()) next_ = Screen::AdminMenu;
}
void ViewAllAppointmentsPanel::update(float) {}
void ViewAllAppointmentsPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); list_.draw(t); btnBack_.draw(t);
}
Screen ViewAllAppointmentsPanel::nextScreen() const { return next_; }

// ============================================================
//  ViewUnpaidBillsPanel
// ============================================================
ViewUnpaidBillsPanel::ViewUnpaidBillsPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::ViewUnpaidBills)
{
    float px = 80.f, py = 70.f, pw = 1120.f, ph = 680.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Unpaid Bills", theme, px + 20.f, py + 15.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 560.f }, theme);
    btnBack_ = Button({ px + 20.f, py + ph - 55.f, 120.f, theme.buttonHeight }, "Back", theme);
}

ViewUnpaidBillsPanel::~ViewUnpaidBillsPanel() {}
void ViewUnpaidBillsPanel::onEnter() { next_ = Screen::ViewUnpaidBills; loadUnpaidBills(); }

void ViewUnpaidBillsPanel::loadUnpaidBills() {
    list_.clear();
    list_.addRow("BillID | Patient Name             | Amount (PKR) | Date", true);

    char today[12]; Utility::getTodayDate(today);
    Bill* arr = gui_->getBills()->getAll();
    int n = gui_->getBills()->size();

    for (int i = 0; i < n; ++i) {
        if (!safeEqual(arr[i].getStatus(), "unpaid")) continue;
        Patient* p = gui_->getPatients()->findById(arr[i].getPatientId());
        const char* pn = p ? p->getName() : "Unknown";
        char row[256]; row[0] = '\0';
        char bid[10], ab[20];
        intToBuf(arr[i].getBillId(), bid); floatToBuf(arr[i].getAmount(), ab);
        safeAppend(row, bid, 256); safeAppend(row, " | ", 256);
        safeAppend(row, pn, 256); safeAppend(row, " | PKR ", 256);
        safeAppend(row, ab, 256); safeAppend(row, " | ", 256);
        safeAppend(row, arr[i].getDate(), 256);
        // Check OVERDUE: date older than 7 days
        // Parse bill date and today, compute difference
        // Format: DD-MM-YYYY  positions: 0-1 day, 3-4 month, 6-9 year
        const char* bd = arr[i].getDate();
        int bDay = (bd[0] - '0') * 10 + (bd[1] - '0');
        int bMon = (bd[3] - '0') * 10 + (bd[4] - '0');
        int bYear = (bd[6] - '0') * 1000 + (bd[7] - '0') * 100 + (bd[8] - '0') * 10 + (bd[9] - '0');
        int tDay = (today[0] - '0') * 10 + (today[1] - '0');
        int tMon = (today[3] - '0') * 10 + (today[4] - '0');
        int tYear = (today[6] - '0') * 1000 + (today[7] - '0') * 100 + (today[8] - '0') * 10 + (today[9] - '0');
        // Convert to days since year 0 (approx)
        long bDays = bYear * 365L + bMon * 30L + bDay;
        long tDays = tYear * 365L + tMon * 30L + tDay;
        if (tDays - bDays > 7) safeAppend(row, " [OVERDUE]", 256);
        list_.addRow(row);
    }
}

void ViewUnpaidBillsPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked; list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnBack_.isClicked()) next_ = Screen::AdminMenu;
}
void ViewUnpaidBillsPanel::update(float) {}
void ViewUnpaidBillsPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); list_.draw(t); btnBack_.draw(t);
}
Screen ViewUnpaidBillsPanel::nextScreen() const { return next_; }

// ============================================================
//  DischargePatientPanel
// ============================================================
DischargePatientPanel::DischargePatientPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), patientCount_(0), next_(Screen::DischargePatient)
{
    float px = 120.f, py = 80.f, pw = 1040.f, ph = 660.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Discharge Patient", theme, px + 20.f, py + 15.f);
    statusMsg_.setFont(theme.regular); statusMsg_.setCharacterSize(theme.fontSizeSmall);
    statusMsg_.setFillColor(theme.errorText);
    statusMsg_.setPosition(px + 20.f, py + ph - 50.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 480.f }, theme);
    btnDischarge_ = Button({ px + 20.f, py + ph - 90.f, 200.f, theme.buttonHeight }, "Discharge Selected", theme);
    btnBack_ = Button({ px + 240.f, py + ph - 90.f, 120.f, theme.buttonHeight }, "Back", theme);
}

DischargePatientPanel::~DischargePatientPanel() {}
void DischargePatientPanel::onEnter() { next_ = Screen::DischargePatient; statusMsg_.setString(""); loadPatients(); }

void DischargePatientPanel::loadPatients() {
    list_.clear(); patientCount_ = 0;
    list_.addRow("ID  | Name                     | Balance     ", true);
    Patient* arr = gui_->getPatients()->getAll();
    int n = gui_->getPatients()->size();
    for (int i = 0; i < n; ++i) {
        char row[256]; row[0] = '\0';
        char idb[10], bb[20];
        intToBuf(arr[i].getId(), idb); floatToBuf(arr[i].getBalance(), bb);
        safeAppend(row, idb, 256); safeAppend(row, " | ", 256);
        safeAppend(row, arr[i].getName(), 256); safeAppend(row, " | PKR ", 256);
        safeAppend(row, bb, 256);
        list_.addRow(row);
        if (patientCount_ < 100) patientIds_[patientCount_++] = arr[i].getId();
    }
    if (patientCount_ == 0) list_.addRow("No patients registered.");
}

void DischargePatientPanel::doDischarge() {
    int sel = list_.getSelectedRow();
    int idx = sel - 1;
    if (sel < 1 || idx >= patientCount_) { statusMsg_.setString("Select a patient."); return; }

    int patId = patientIds_[idx];
    // Check unpaid bills
    Bill* bills = gui_->getBills()->getAll();
    int bn = gui_->getBills()->size();
    for (int i = 0; i < bn; ++i) {
        if (bills[i].getPatientId() == patId && safeEqual(bills[i].getStatus(), "unpaid")) {
            statusMsg_.setString("Cannot discharge patient with unpaid bills.");
            return;
        }
    }
    // Check pending appointments
    Appointment* arr = gui_->getAppointments()->getAll();
    int an = gui_->getAppointments()->size();
    for (int i = 0; i < an; ++i) {
        if (arr[i].getPatientId() == patId && safeEqual(arr[i].getStatus(), "pending")) {
            statusMsg_.setString("Cannot discharge patient with pending appointments.");
            return;
        }
    }

    Patient* pat = gui_->getPatients()->findById(patId);
    if (!pat) { statusMsg_.setString("Patient not found."); return; }

    FileHandler::archivePatient(*pat, *gui_->getAppointments(),
        *gui_->getPrescriptions(), *gui_->getBills());
    FileHandler::deleteAppointmentsByPatient(patId, *gui_->getAppointments());
    FileHandler::deletePrescriptionsByPatient(patId, *gui_->getPrescriptions());
    FileHandler::deleteBillsByPatient(patId, *gui_->getBills());
    FileHandler::deletePatient(patId, *gui_->getPatients());

    gui_->showMessage("Patient discharged and archived successfully.", Screen::AdminMenu, false);
}

void DischargePatientPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked; list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnDischarge_.update(mpos, mouseDown); btnBack_.update(mpos, mouseDown);
    if (btnDischarge_.isClicked()) doDischarge();
    if (btnBack_.isClicked()) next_ = Screen::AdminMenu;
}
void DischargePatientPanel::update(float) {}
void DischargePatientPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); t.draw(statusMsg_);
    list_.draw(t); btnDischarge_.draw(t); btnBack_.draw(t);
}
Screen DischargePatientPanel::nextScreen() const { return next_; }

// ============================================================
//  SecurityLogPanel
// ============================================================
SecurityLogPanel::SecurityLogPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::SecurityLog)
{
    float px = 80.f, py = 70.f, pw = 1120.f, ph = 680.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Security Log", theme, px + 20.f, py + 15.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 560.f }, theme);
    btnBack_ = Button({ px + 20.f, py + ph - 55.f, 120.f, theme.buttonHeight }, "Back", theme);
}

SecurityLogPanel::~SecurityLogPanel() {}
void SecurityLogPanel::onEnter() { next_ = Screen::SecurityLog; loadLog(); }

void SecurityLogPanel::loadLog() {
    list_.clear();
    list_.addRow("Timestamp              | Role    | Entered ID | Result", true);

    FILE* f = fopen("security_log.txt", "r");
    if (!f) { list_.addRow("No security events logged."); return; }

    char line[300]; bool any = false;
    while (fgets(line, 300, f)) {
        // Strip trailing newline
        int len = 0;
        while (line[len] != '\0') ++len;
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }
        if (len > 0) { list_.addRow(line); any = true; }
    }
    fclose(f);
    if (!any) list_.addRow("No security events logged.");
}

void SecurityLogPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked; list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnBack_.isClicked()) next_ = Screen::AdminMenu;
}
void SecurityLogPanel::update(float) {}
void SecurityLogPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); list_.draw(t); btnBack_.draw(t);
}
Screen SecurityLogPanel::nextScreen() const { return next_; }

// ============================================================
//  DailyReportPanel
// ============================================================
DailyReportPanel::DailyReportPanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), next_(Screen::DailyReport)
{
    float px = 60.f, py = 60.f, pw = 1160.f, ph = 700.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    heading_ = makeHeading("Daily Report", theme, px + 20.f, py + 15.f);
    list_ = ScrollableList({ px + 20.f, py + 60.f, pw - 40.f, 580.f }, theme);
    btnBack_ = Button({ px + 20.f, py + ph - 55.f, 120.f, theme.buttonHeight }, "Back", theme);
}

DailyReportPanel::~DailyReportPanel() {}
void DailyReportPanel::onEnter() { next_ = Screen::DailyReport; buildReport(); }

void DailyReportPanel::buildReport() {
    list_.clear();
    char today[12]; Utility::getTodayDate(today);
    char title[60] = "Daily Report — "; safeAppend(title, today, 60);
    list_.addRow(title, true);
    list_.addRow("");

    // Count today's appointments by status
    Appointment* arr = gui_->getAppointments()->getAll();
    int n = gui_->getAppointments()->size();
    int total = 0, pending = 0, completed = 0, noshow = 0, cancelled = 0;
    for (int i = 0; i < n; ++i) {
        if (!safeEqual(arr[i].getDate(), today)) continue;
        ++total;
        if (safeEqual(arr[i].getStatus(), "pending"))   ++pending;
        else if (safeEqual(arr[i].getStatus(), "completed")) ++completed;
        else if (safeEqual(arr[i].getStatus(), "noshow"))    ++noshow;
        else if (safeEqual(arr[i].getStatus(), "cancelled")) ++cancelled;
    }
    char apptRow[120] = "Appointments today: Total=";
    char nb[8]; intToBuf(total, nb); safeAppend(apptRow, nb, 120);
    safeAppend(apptRow, "  Pending=", 120); intToBuf(pending, nb); safeAppend(apptRow, nb, 120);
    safeAppend(apptRow, "  Completed=", 120); intToBuf(completed, nb); safeAppend(apptRow, nb, 120);
    safeAppend(apptRow, "  No-show=", 120); intToBuf(noshow, nb); safeAppend(apptRow, nb, 120);
    safeAppend(apptRow, "  Cancelled=", 120); intToBuf(cancelled, nb); safeAppend(apptRow, nb, 120);
    list_.addRow(apptRow);

    // Revenue from paid bills today
    Bill* bills = gui_->getBills()->getAll();
    int bn = gui_->getBills()->size();
    float revenue = 0.f;
    for (int i = 0; i < bn; ++i) {
        if (safeEqual(bills[i].getDate(), today) && safeEqual(bills[i].getStatus(), "paid"))
            revenue += bills[i].getAmount();
    }
    char revRow[60] = "Revenue collected today: PKR ";
    char rb[20]; floatToBuf(revenue, rb); safeAppend(revRow, rb, 60);
    list_.addRow(revRow);
    list_.addRow("");

    // Patients with outstanding unpaid bills
    list_.addRow("Patients with unpaid bills:", true);
    Patient* patients = gui_->getPatients()->getAll();
    int pn = gui_->getPatients()->size();
    for (int i = 0; i < pn; ++i) {
        float owed = 0.f;
        for (int j = 0; j < bn; ++j)
            if (bills[j].getPatientId() == patients[i].getId() &&
                safeEqual(bills[j].getStatus(), "unpaid"))
                owed += bills[j].getAmount();
        if (owed > 0.f) {
            char pr[120]; pr[0] = '\0';
            safeAppend(pr, patients[i].getName(), 120); safeAppend(pr, " — PKR ", 120);
            char ob[20]; floatToBuf(owed, ob); safeAppend(pr, ob, 120);
            list_.addRow(pr);
        }
    }
    list_.addRow("");

    // Doctor-wise summary for today
    list_.addRow("Doctor Summary (today):", true);
    Doctor* doctors = gui_->getDoctors()->getAll();
    int dn = gui_->getDoctors()->size();
    for (int i = 0; i < dn; ++i) {
        int dc = 0, dp = 0, dns_ = 0;
        for (int j = 0; j < n; ++j) {
            if (arr[j].getDoctorId() != doctors[i].getId()) continue;
            if (!safeEqual(arr[j].getDate(), today)) continue;
            if (safeEqual(arr[j].getStatus(), "completed")) ++dc;
            else if (safeEqual(arr[j].getStatus(), "pending"))  ++dp;
            else if (safeEqual(arr[j].getStatus(), "noshow"))   ++dns_;
        }
        if (dc + dp + dns_ == 0) continue;
        char dr[150]; dr[0] = '\0';
        safeAppend(dr, doctors[i].getName(), 150);
        safeAppend(dr, "  Completed=", 150); char c[6]; intToBuf(dc, c); safeAppend(dr, c, 150);
        safeAppend(dr, "  Pending=", 150); intToBuf(dp, c); safeAppend(dr, c, 150);
        safeAppend(dr, "  No-show=", 150); intToBuf(dns_, c); safeAppend(dr, c, 150);
        list_.addRow(dr);
    }
}

void DailyReportPanel::handleEvent(const sf::Event& ev, sf::Vector2f mpos,
    bool clicked, bool mouseDown) {
    (void)clicked; list_.handleEvent(ev); list_.update(mpos, mouseDown);
    btnBack_.update(mpos, mouseDown);
    if (btnBack_.isClicked()) next_ = Screen::AdminMenu;
}
void DailyReportPanel::update(float) {}
void DailyReportPanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(heading_); list_.draw(t); btnBack_.draw(t);
}
Screen DailyReportPanel::nextScreen() const { return next_; }

// ============================================================
//  MessagePanel
// ============================================================
MessagePanel::MessagePanel(GUI* gui, const Theme& theme)
    : gui_(gui), theme_(&theme), returnTo_(Screen::RoleSelect),
    isError_(false), next_(Screen::Message)
{
    message_[0] = '\0';
    float px = 340.f, py = 300.f, pw = 600.f, ph = 200.f;
    panel_ = makePanel(px, py, pw, ph, theme);
    msgText_.setFont(theme.regular);
    msgText_.setCharacterSize(theme.fontSizeBody);
    msgText_.setPosition(px + 30.f, py + 40.f);

    btnOk_ = Button({ px + pw / 2.f - 80.f, py + ph - 60.f, 160.f, theme.buttonHeight },
        "OK", theme);
}

MessagePanel::~MessagePanel() {}

void MessagePanel::setMessage(const char* msg, Screen ret, bool isErr) {
    safeCopy(message_, msg, 300);
    returnTo_ = ret;
    isError_ = isErr;
}

void MessagePanel::onEnter() {
    next_ = Screen::Message;
    msgText_.setString(message_);
    msgText_.setFillColor(isError_ ? theme_->errorText : theme_->successText);
}

void MessagePanel::handleEvent(const sf::Event&, sf::Vector2f mpos,
    bool, bool mouseDown) {
    btnOk_.update(mpos, mouseDown);
    if (btnOk_.isClicked()) next_ = returnTo_;
}
void MessagePanel::update(float) {}
void MessagePanel::draw(sf::RenderTarget& t) const {
    t.draw(panel_); t.draw(msgText_); btnOk_.draw(t);
}
Screen MessagePanel::nextScreen() const { return next_; }