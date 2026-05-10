#pragma once
#ifndef GUI_H
#define GUI_H

// ============================================================
// GUI.h
// SFML 2.6.2 graphical front-end for the MediCore Hospital
// Management System.
//
// Replaces all console-based menus (Menu.h / Menu.cpp) with a
// fully windowed UI rendered through SFML.
//
// Architecture overview:
//   - GUI is the central controller; it owns the sf::RenderWindow
//     and drives the event/render loop.
//   - All application data (Storage<T>, Admin) is passed in by
//     pointer at construction time — GUI never owns them.
//   - Screen state is managed by a Screen enum and one active
//     UIPanel pointer (polymorphic panels handle their own layout).
//   - All text input is handled by InputBox; all clickable
//     elements are Button.
//   - No std::string anywhere — every label / buffer is a
//     plain char array.
//   - No global variables; everything lives inside GUI or one
//     of the panel classes.
//
// SFML modules used:  Graphics, Window, System
// SFML version:       2.6.2
//
// Typical call from main():
//   GUI gui(&patients, &doctors, &appointments, &bills,
//           &prescriptions, admin);
//   gui.run();
// ============================================================

#include <SFML/Graphics.hpp>

#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"

// ============================================================
// Forward declarations of all panel classes
// (defined later in this header; implemented in GUI.cpp)
// ============================================================
class UIPanel;
class LoginPanel;
class PatientMenuPanel;
class BookAppointmentPanel;
class CancelAppointmentPanel;
class ViewAppointmentsPanel;
class ViewMedicalRecordsPanel;
class ViewBillsPanel;
class PayBillPanel;
class TopUpPanel;
class DoctorMenuPanel;
class ViewTodayPanel;
class MarkCompletedPanel;
class MarkNoShowPanel;
class WritePrescriptionPanel;
class ViewPatientHistoryPanel;
class AdminMenuPanel;
class AddDoctorPanel;
class RemoveDoctorPanel;
class ViewAllPatientsPanel;
class ViewAllDoctorsPanel;
class ViewAllAppointmentsPanel;
class ViewUnpaidBillsPanel;
class DischargePatientPanel;
class SecurityLogPanel;
class DailyReportPanel;
class MessagePanel;

// ============================================================
// Screen — identifies which "page" is currently active.
// GUI::switchTo() transitions between them.
// ============================================================
enum class Screen {
    // -- Entry --
    RoleSelect,         // "Login as: Patient / Doctor / Admin / Exit"

    // -- Login --
    PatientLogin,
    DoctorLogin,
    AdminLogin,

    // -- Patient screens --
    PatientMenu,
    BookAppointment,
    CancelAppointment,
    ViewMyAppointments,
    ViewMedicalRecords,
    ViewMyBills,
    PayBill,
    TopUpBalance,

    // -- Doctor screens --
    DoctorMenu,
    ViewToday,
    MarkCompleted,
    MarkNoShow,
    WritePrescription,
    ViewPatientHistory,

    // -- Admin screens --
    AdminMenu,
    AddDoctor,
    RemoveDoctor,
    ViewAllPatients,
    ViewAllDoctors,
    ViewAllAppointments,
    ViewUnpaidBills,
    DischargePatient,
    SecurityLog,
    DailyReport,

    // -- Utility --
    Message,            // generic one-line feedback panel
    Exit
};

// ============================================================
// Theme — colour/font constants shared by every widget.
// All values are set in GUI::initTheme() from a single place.
// Declared as a class (all members public) to avoid 'struct'.
// ============================================================
class Theme {
public:
    // -- Colours --
    sf::Color background;       // window clear colour
    sf::Color panelBg;          // panel / card background
    sf::Color panelBorder;      // panel outline
    sf::Color buttonNormal;     // button fill (idle)
    sf::Color buttonHover;      // button fill (mouse over)
    sf::Color buttonActive;     // button fill (clicked)
    sf::Color buttonText;       // text on buttons
    sf::Color inputBg;          // text-input box background
    sf::Color inputBorder;      // text-input box border
    sf::Color inputBorderFocus; // text-input border when focused
    sf::Color inputText;        // typed text colour
    sf::Color labelText;        // ordinary label colour
    sf::Color headingText;      // section heading colour
    sf::Color errorText;        // error / warning messages
    sf::Color successText;      // success feedback
    sf::Color tableHeader;      // table column-header background
    sf::Color tableRowEven;     // table even-row background
    sf::Color tableRowOdd;      // table odd-row background
    sf::Color tableText;        // table cell text colour
    sf::Color scrollBar;        // scrollbar fill
    sf::Color highlight;        // selected item in a list

    // -- Fonts --
    sf::Font  regular;          // body text font
    sf::Font  bold;             // headings / button labels

    // -- Sizes --
    unsigned int fontSizeBody;      // default body text (px)
    unsigned int fontSizeHeading;   // panel heading (px)
    unsigned int fontSizeSmall;     // small caption (px)
    unsigned int fontSizeButton;    // button label (px)

    float       buttonRadius;       // rounded-corner radius (px)
    float       padding;            // universal inner padding (px)
    float       inputHeight;        // fixed height of InputBox (px)
    float       buttonHeight;       // fixed height of Button (px)
    float       borderThickness;    // outline thickness (px)
};

// ============================================================
// Button
// A labelled, clickable rectangle with hover and click states.
// ============================================================
class Button {
public:
    // Construct with bounding rect and label text.
    // 'theme' must outlive this Button.
    Button();
    Button(const sf::FloatRect& bounds,
        const char* label,
        const Theme& theme);

    // Update hover/active state from current mouse position.
    // Call every frame before draw().
    void update(sf::Vector2f mousePos, bool mouseDown);

    // Draw to the target.
    void draw(sf::RenderTarget& target) const;

    // Returns true if the button was clicked this frame
    // (mouse released while cursor is inside).
    bool isClicked() const;

    // Reposition or resize at runtime (e.g. after window resize).
    void setBounds(const sf::FloatRect& bounds);

    // Change the label text (e.g. toggle buttons).
    void setLabel(const char* label);

    // Enable / disable. Disabled buttons are greyed out and
    // do not respond to input.
    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Retrieve the current bounding rect.
    sf::FloatRect getBounds() const;

private:
    sf::RectangleShape shape_;
    sf::Text           label_;
    const Theme* theme_;
    bool               hovered_;
    bool               pressed_;      // mouse is held down over button
    bool               clicked_;      // true for exactly one frame
    bool               enabled_;
    bool               prevMouseDown_; // edge-detection for click
};

// ============================================================
// InputBox
// A single-line text input field.
// Supports cursor, backspace, printable ASCII entry.
// ============================================================
class InputBox {
public:
    // maxLen is the capacity of the internal buffer (including \0).
    InputBox();
    InputBox(const sf::FloatRect& bounds,
        const char* placeholder,
        const Theme& theme,
        int                  maxLen = 100,
        bool                 masked = false); // true for passwords

    // Rule of Three: InputBox owns a heap-allocated char* buffer.
    // Without these, the compiler's generated copy/assign would
    // shallow-copy the pointer causing double-free and C2248 via
    // sf::Text (NonCopyable).  We deep-copy the buffer explicitly.
    InputBox(const InputBox& o);
    InputBox& operator=(const InputBox& o);
    ~InputBox();

    // Forward keyboard events from the window event loop.
    void handleEvent(const sf::Event& event);

    // Update focus state (click to focus).
    void update(sf::Vector2f mousePos, bool mouseClicked);

    // Draw to target.
    void draw(sf::RenderTarget& target) const;

    // Retrieve the current text (null-terminated char array).
    const char* getText() const;

    // Clear the contents.
    void clear();

    // Programmatically set focus.
    void setFocus(bool f);
    bool hasFocus() const;

    // Reposition.
    void setBounds(const sf::FloatRect& bounds);

    sf::FloatRect getBounds() const;

private:
    sf::RectangleShape shape_;
    sf::Text           displayText_;
    sf::Text           placeholder_;
    sf::RectangleShape cursor_;

    const Theme* theme_;
    char* buffer_;     // dynamically allocated, size = maxLen_
    int                maxLen_;
    int                length_;     // current char count (excluding \0)
    bool               focused_;
    bool               masked_;     // if true, show '*' instead of chars

    sf::Clock          cursorClock_;
    bool               cursorVisible_;

    // Rebuild displayText_ content (respects masked_).
    void rebuildDisplay();
};

// ============================================================
// Label
// A non-interactive piece of text.
// ============================================================
class Label {
public:
    Label();
    Label(sf::Vector2f     position,
        const char* text,
        const Theme& theme,
        unsigned int     fontSize = 0,    // 0 = use theme body size
        sf::Color        colour = sf::Color::Transparent, // Transparent = use theme label colour
        bool             bold = false);

    void draw(sf::RenderTarget& target) const;

    void setText(const char* text);
    void setPosition(sf::Vector2f pos);
    void setColour(sf::Color c);

private:
    sf::Text     text_;
    const Theme* theme_;
};

// ============================================================
// ScrollableList
// A vertically scrollable list of selectable text rows.
// Used for appointment lists, doctor lists, bill lists, etc.
// ============================================================
class ScrollableList {
public:
    // maxRows: maximum number of rows that can be stored.
    ScrollableList();
    ScrollableList(const sf::FloatRect& bounds,
        const Theme& theme,
        int                  maxRows = 100);

    ~ScrollableList();

    // Clear all rows.
    void clear();

    // Append a row.  'text' is copied into an internal buffer.
    void addRow(const char* text, bool isHeader = false);

    // Handle scroll wheel and click events.
    void handleEvent(const sf::Event& event);
    void update(sf::Vector2f mousePos, bool mouseClicked);

    // Draw.
    void draw(sf::RenderTarget& target) const;

    // Returns the index of the currently selected row,
    // or -1 if nothing is selected.
    int getSelectedRow() const;

    // Retrieve text of a specific row (returns empty string if out of range).
    const char* getRowText(int index) const;

    int rowCount() const;

private:
    sf::FloatRect       bounds_;
    const Theme* theme_;

    // Row storage — heap-allocated arrays of char[256]
    char** rows_;
    bool* isHeader_;  // header rows are styled differently
    int                 maxRows_;
    int                 rowCount_;

    int                 selectedRow_;
    int                 scrollOffset_;  // top visible row index

    float               rowHeight_;     // computed from font size + padding

    sf::RectangleShape  background_;
    sf::RectangleShape  scrollBarShape_;

    // How many rows fit in the visible area.
    int visibleRowCount() const;

    // Rebuild the scrollbar geometry.
    void updateScrollBar();
};

// ============================================================
// Panel (abstract)
// Each "page" in the application is a Panel.
// GUI holds one active Panel* and delegates events/updates/draws.
// ============================================================
class UIPanel {
public:
    virtual ~UIPanel() {}

    // Called once per event from the SFML event loop.
    virtual void handleEvent(const sf::Event& event,
        sf::Vector2f     mousePos,
        bool             mouseClicked,
        bool             mouseDown) = 0;

    // Called once per frame (before draw).
    virtual void update(float dt) = 0;

    // Called once per frame to render.
    virtual void draw(sf::RenderTarget& target) const = 0;

    // Returns the Screen the GUI should switch to, or the current
    // screen if no transition is needed yet.
    virtual Screen nextScreen() const = 0;

    // Called whenever the GUI switches TO this panel, so it can
    // refresh its data from the live Storage<T> objects.
    virtual void onEnter() {}

    // Called whenever the GUI switches AWAY from this panel.
    virtual void onExit() {}
};

// ============================================================
// GUI — main class
// Owns the window, the Theme, and every Panel.
// ============================================================
class GUI {
public:
    // -----------------------------------------------------------
    // Constructor
    // Receives pointers to all live data stores.
    // Does NOT take ownership — the caller keeps the stores alive.
    // -----------------------------------------------------------
    GUI(Storage<Patient>* patients,
        Storage<Doctor>* doctors,
        Storage<Appointment>* appointments,
        Storage<Bill>* bills,
        Storage<Prescription>* prescriptions,
        Admin& admin);

    // -----------------------------------------------------------
    // Destructor — deletes all panels and the window.
    // -----------------------------------------------------------
    ~GUI();

    // -----------------------------------------------------------
    // run()
    // Opens the window and enters the event/update/render loop.
    // Returns when the user exits (Screen::Exit or window closed).
    // -----------------------------------------------------------
    void run();

    // -----------------------------------------------------------
    // switchTo()
    // Transitions to the given screen.  Calls onExit() on the old
    // panel and onEnter() on the new one.
    // Callable from within panels via a pointer to GUI.
    // -----------------------------------------------------------
    void switchTo(Screen screen);

    // -----------------------------------------------------------
    // showMessage()
    // Displays a transient one-line message (success / error)
    // and then returns to 'returnTo' when the user dismisses it.
    // -----------------------------------------------------------
    void showMessage(const char* message,
        Screen      returnTo,
        bool        isError = false);

    // -----------------------------------------------------------
    // Accessors — panels call these to reach shared data.
    // -----------------------------------------------------------
    Storage<Patient>* getPatients()      const;
    Storage<Doctor>* getDoctors()       const;
    Storage<Appointment>* getAppointments()  const;
    Storage<Bill>* getBills()         const;
    Storage<Prescription>* getPrescriptions() const;
    Admin& getAdmin()         const;
    const Theme& getTheme()         const;

    // -----------------------------------------------------------
    // Session state — set after successful login, cleared on logout.
    // -----------------------------------------------------------
    void    setLoggedInPatient(Patient* p);
    void    setLoggedInDoctor(Doctor* d);
    Patient* getLoggedInPatient() const;
    Doctor* getLoggedInDoctor()  const;
    void    clearSession();

    // Failed-login counter (resets on successful login).
    void incrementFailedLogins(const char* role, const char* enteredId);
    void resetFailedLogins();
    int  failedLoginCount() const;

private:
    // -----------------------------------------------------------
    // Window & rendering
    // -----------------------------------------------------------
    sf::RenderWindow* window_;
    sf::Vector2u      windowSize_;   // 1280 x 800

    // -----------------------------------------------------------
    // Theme
    // -----------------------------------------------------------
    Theme theme_;
    void  initTheme();   // loads fonts, sets colours, sets sizes

    // -----------------------------------------------------------
    // Application data (not owned)
    // -----------------------------------------------------------
    Storage<Patient>* patients_;
    Storage<Doctor>* doctors_;
    Storage<Appointment>* appointments_;
    Storage<Bill>* bills_;
    Storage<Prescription>* prescriptions_;
    Admin* admin_;

    // -----------------------------------------------------------
    // Session
    // -----------------------------------------------------------
    Patient* loggedInPatient_;
    Doctor* loggedInDoctor_;
    int      failedLogins_;

    // -----------------------------------------------------------
    // Panel registry
    // One panel object per Screen value that has a panel.
    // Panels are created once in initPanels() and reused.
    // -----------------------------------------------------------
    UIPanel* panels_[64];   // indexed by (int)Screen
    Screen   currentScreen_;

    void initPanels();
    void destroyPanels();

    UIPanel* activePanel() const;

    // -----------------------------------------------------------
    // Per-frame helpers
    // -----------------------------------------------------------
    void processEvents();
    void update(float dt);
    void render();

    // The message panel needs extra state:
    Screen messageReturnTo_;

    // -----------------------------------------------------------
    // Copy prevention
    // -----------------------------------------------------------
    GUI(const GUI&);
    GUI& operator=(const GUI&);
};

// ============================================================
//  PANEL DECLARATIONS
//  Each panel corresponds to one Screen value.
//  All panels accept a GUI* so they can call GUI::switchTo(),
//  GUI::showMessage(), and access the shared data stores.
// ============================================================

// ------------------------------------------------------------
// RoleSelectPanel  →  Screen::RoleSelect
// Shows: "Login as: 1.Patient  2.Doctor  3.Admin  4.Exit"
// ------------------------------------------------------------
class RoleSelectPanel : public UIPanel {
public:
    explicit RoleSelectPanel(GUI* gui, const Theme& theme);
    ~RoleSelectPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    // Background gradient rectangle
    sf::RectangleShape bgTop_;
    sf::RectangleShape bgBottom_;

    // Hospital logo text (rendered as sf::Text with large font)
    sf::Text    logo_;
    sf::Text    subtitle_;

    Button      btnPatient_;
    Button      btnDoctor_;
    Button      btnAdmin_;
    Button      btnExit_;

    Screen      next_;
};

// ------------------------------------------------------------
// LoginPanel  →  Screen::PatientLogin / DoctorLogin / AdminLogin
// Generic login form: ID field + Password field + Login button.
// The role (Patient / Doctor / Admin) is set at construction.
// ------------------------------------------------------------
class LoginPanel : public UIPanel {
public:
    // role: "Patient", "Doctor", or "Admin"
    LoginPanel(GUI* gui, const Theme& theme, const char* role);
    ~LoginPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;
    char         role_[16];    // "Patient" / "Doctor" / "Admin"

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           errorMsg_;

    Label    lblId_;
    Label    lblPassword_;
    InputBox inputId_;
    InputBox inputPassword_;
    Button   btnLogin_;
    Button   btnBack_;

    Screen   next_;

    // Attempt login using the current content of inputId_ / inputPassword_.
    void attemptLogin();

    // Returns Screen::PatientMenu / DoctorMenu / AdminMenu for this role.
    Screen successScreen() const;

    // Returns Screen::PatientLogin / DoctorLogin / AdminLogin for this role.
    Screen selfScreen() const;
};

// ------------------------------------------------------------
// PatientMenuPanel  →  Screen::PatientMenu
// Shows the 8 patient menu items as buttons.
// Displays patient name and balance in the header.
// ------------------------------------------------------------
class PatientMenuPanel : public UIPanel {
public:
    PatientMenuPanel(GUI* gui, const Theme& theme);
    ~PatientMenuPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape headerBar_;
    sf::Text           lblWelcome_;
    sf::Text           lblBalance_;

    Button  btnBook_;
    Button  btnCancel_;
    Button  btnViewAppts_;
    Button  btnMedRecords_;
    Button  btnBills_;
    Button  btnPayBill_;
    Button  btnTopUp_;
    Button  btnLogout_;

    Screen  next_;

    void refreshHeader();
};

// ------------------------------------------------------------
// BookAppointmentPanel  →  Screen::BookAppointment
// Multi-step form: specialization search → doctor select →
// date entry → slot select → confirmation.
// ------------------------------------------------------------
class BookAppointmentPanel : public UIPanel {
public:
    BookAppointmentPanel(GUI* gui, const Theme& theme);
    ~BookAppointmentPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    // Step tracker (0 = spec search, 1 = doctor list,
    //               2 = date entry, 3 = slot select, 4 = confirm)
    int step_;

    // -- Step 0: specialization search --
    sf::RectangleShape panel_;
    sf::Text           heading_;
    Label              lblSpec_;
    InputBox           inputSpec_;
    Button             btnSearchSpec_;
    Button             btnBack_;

    // -- Step 1: doctor list --
    ScrollableList     doctorList_;
    Button             btnSelectDoctor_;

    // Stores matching doctor indices into doctors store
    int    matchedDoctorIndices_[100];
    int    matchedDoctorCount_;

    // Selected doctor (pointer into Storage, not owned)
    Doctor* selectedDoctor_;

    // -- Step 2: date entry --
    Label    lblDate_;
    InputBox inputDate_;
    Button   btnConfirmDate_;
    sf::Text dateError_;
    int      dateAttempts_;

    char     selectedDate_[12]; // "DD-MM-YYYY"

    // -- Step 3: slot selection --
    // 8 slot buttons
    Button   slotButtons_[8];
    sf::Text slotLabels_[8];   // "09:00" .. "16:00"
    bool     slotAvailable_[8];
    sf::Text slotError_;
    char     selectedSlot_[6];

    // -- Step 4: confirmation panel --
    sf::Text confirmText_;
    Button   btnConfirm_;
    Button   btnCancelBook_;

    Screen  next_;

    // Helpers
    void doSpecSearch();
    void buildDoctorList();
    void buildSlotGrid();
    bool computeSlotAvailability();
    void doBooking();
    void resetToStep(int s);

    static const char* SLOTS[8];
};

// ------------------------------------------------------------
// CancelAppointmentPanel  →  Screen::CancelAppointment
// Lists pending appointments; user selects one to cancel.
// ------------------------------------------------------------
class CancelAppointmentPanel : public UIPanel {
public:
    CancelAppointmentPanel(GUI* gui, const Theme& theme);
    ~CancelAppointmentPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           statusMsg_;

    ScrollableList     apptList_;

    // Appointment IDs parallel to list rows (index → appointment ID)
    int   apptIds_[100];
    int   apptCount_;

    Button  btnCancel_;    // "Cancel Selected"
    Button  btnBack_;

    Screen  next_;

    void loadPendingAppointments();
    void doCancellation();
};

// ------------------------------------------------------------
// ViewAppointmentsPanel  →  Screen::ViewMyAppointments
// Shows all appointments sorted ascending by date.
// ------------------------------------------------------------
class ViewAppointmentsPanel : public UIPanel {
public:
    ViewAppointmentsPanel(GUI* gui, const Theme& theme);
    ~ViewAppointmentsPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    ScrollableList     list_;
    Button             btnBack_;
    Screen             next_;

    void loadAppointments();
};

// ------------------------------------------------------------
// ViewMedicalRecordsPanel  →  Screen::ViewMedicalRecords
// Groups prescriptions by appointment, sorted date descending.
// ------------------------------------------------------------
class ViewMedicalRecordsPanel : public UIPanel {
public:
    ViewMedicalRecordsPanel(GUI* gui, const Theme& theme);
    ~ViewMedicalRecordsPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    ScrollableList     list_;
    Button             btnBack_;
    Screen             next_;

    void loadRecords();
};

// ------------------------------------------------------------
// ViewBillsPanel  →  Screen::ViewMyBills
// Shows all bills and total outstanding amount.
// ------------------------------------------------------------
class ViewBillsPanel : public UIPanel {
public:
    ViewBillsPanel(GUI* gui, const Theme& theme);
    ~ViewBillsPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           totalOwed_;
    ScrollableList     list_;
    Button             btnBack_;
    Screen             next_;

    void loadBills();
};

// ------------------------------------------------------------
// PayBillPanel  →  Screen::PayBill
// Shows unpaid bills; user selects one to pay.
// ------------------------------------------------------------
class PayBillPanel : public UIPanel {
public:
    PayBillPanel(GUI* gui, const Theme& theme);
    ~PayBillPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           statusMsg_;
    ScrollableList     list_;

    int  billIds_[100];
    int  billCount_;

    Button  btnPay_;
    Button  btnBack_;
    Screen  next_;

    void loadUnpaidBills();
    void doPayment();
};

// ------------------------------------------------------------
// TopUpPanel  →  Screen::TopUpBalance
// Text field for amount + button; validates > 0.
// ------------------------------------------------------------
class TopUpPanel : public UIPanel {
public:
    TopUpPanel(GUI* gui, const Theme& theme);
    ~TopUpPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           errorMsg_;
    sf::Text           balanceInfo_;

    Label    lblAmount_;
    InputBox inputAmount_;
    Button   btnTopUp_;
    Button   btnBack_;
    int      attempts_;
    Screen   next_;

    void doTopUp();
};

// ============================================================
// DOCTOR PANELS
// ============================================================

// ------------------------------------------------------------
// DoctorMenuPanel  →  Screen::DoctorMenu
// ------------------------------------------------------------
class DoctorMenuPanel : public UIPanel {
public:
    DoctorMenuPanel(GUI* gui, const Theme& theme);
    ~DoctorMenuPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape headerBar_;
    sf::Text           lblWelcome_;

    Button  btnViewToday_;
    Button  btnMarkCompleted_;
    Button  btnMarkNoShow_;
    Button  btnWriteRx_;
    Button  btnPatientHistory_;
    Button  btnLogout_;

    Screen  next_;
};

// ------------------------------------------------------------
// ViewTodayPanel  →  Screen::ViewToday
// ------------------------------------------------------------
class ViewTodayPanel : public UIPanel {
public:
    ViewTodayPanel(GUI* gui, const Theme& theme);
    ~ViewTodayPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    ScrollableList     list_;
    Button             btnBack_;
    Screen             next_;

    void loadTodayAppointments();
};

// ------------------------------------------------------------
// MarkCompletedPanel  →  Screen::MarkCompleted
// ------------------------------------------------------------
class MarkCompletedPanel : public UIPanel {
public:
    MarkCompletedPanel(GUI* gui, const Theme& theme);
    ~MarkCompletedPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           statusMsg_;
    ScrollableList     list_;

    int apptIds_[100];
    int apptCount_;

    Button  btnMark_;
    Button  btnBack_;
    Screen  next_;

    void loadPendingTodayAppointments();
    void doMarkCompleted();
};

// ------------------------------------------------------------
// MarkNoShowPanel  →  Screen::MarkNoShow
// ------------------------------------------------------------
class MarkNoShowPanel : public UIPanel {
public:
    MarkNoShowPanel(GUI* gui, const Theme& theme);
    ~MarkNoShowPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           statusMsg_;
    ScrollableList     list_;

    int apptIds_[100];
    int apptCount_;

    Button  btnMark_;
    Button  btnBack_;
    Screen  next_;

    void loadPendingTodayAppointments();
    void doMarkNoShow();
};

// ------------------------------------------------------------
// WritePrescriptionPanel  →  Screen::WritePrescription
// ------------------------------------------------------------
class WritePrescriptionPanel : public UIPanel {
public:
    WritePrescriptionPanel(GUI* gui, const Theme& theme);
    ~WritePrescriptionPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           statusMsg_;

    // Step 0: enter appointment ID
    Label    lblApptId_;
    InputBox inputApptId_;
    Button   btnFindAppt_;

    // Step 1: enter medicines and notes
    Label    lblMedicines_;
    InputBox inputMedicines_;   // up to 499 chars
    Label    lblNotes_;
    InputBox inputNotes_;       // up to 299 chars
    Button   btnSaveRx_;
    Button   btnBack_;

    int    step_;
    int    selectedApptId_;
    Screen next_;

    void doFindAppointment();
    void doSavePrescription();
};

// ------------------------------------------------------------
// ViewPatientHistoryPanel  →  Screen::ViewPatientHistory
// ------------------------------------------------------------
class ViewPatientHistoryPanel : public UIPanel {
public:
    ViewPatientHistoryPanel(GUI* gui, const Theme& theme);
    ~ViewPatientHistoryPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           statusMsg_;

    Label    lblPatientId_;
    InputBox inputPatientId_;
    Button   btnSearch_;

    ScrollableList list_;
    Button         btnBack_;

    Screen next_;

    void doSearch();
};

// ============================================================
// ADMIN PANELS
// ============================================================

// ------------------------------------------------------------
// AdminMenuPanel  →  Screen::AdminMenu
// ------------------------------------------------------------
class AdminMenuPanel : public UIPanel {
public:
    AdminMenuPanel(GUI* gui, const Theme& theme);
    ~AdminMenuPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape headerBar_;
    sf::Text           lblHeading_;

    Button  btnAddDoctor_;
    Button  btnRemoveDoctor_;
    Button  btnAllPatients_;
    Button  btnAllDoctors_;
    Button  btnAllAppts_;
    Button  btnUnpaidBills_;
    Button  btnDischarge_;
    Button  btnSecLog_;
    Button  btnReport_;
    Button  btnLogout_;

    Screen  next_;
};

// ------------------------------------------------------------
// AddDoctorPanel  →  Screen::AddDoctor
// Form: name, specialization, contact, password, fee.
// ------------------------------------------------------------
class AddDoctorPanel : public UIPanel {
public:
    AddDoctorPanel(GUI* gui, const Theme& theme);
    ~AddDoctorPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           errorMsg_;

    Label    lblName_;
    Label    lblSpec_;
    Label    lblContact_;
    Label    lblPassword_;
    Label    lblFee_;

    InputBox inputName_;
    InputBox inputSpec_;
    InputBox inputContact_;
    InputBox inputPassword_;
    InputBox inputFee_;

    Button  btnAdd_;
    Button  btnBack_;

    Screen  next_;

    void doAddDoctor();
};

// ------------------------------------------------------------
// RemoveDoctorPanel  →  Screen::RemoveDoctor
// ------------------------------------------------------------
class RemoveDoctorPanel : public UIPanel {
public:
    RemoveDoctorPanel(GUI* gui, const Theme& theme);
    ~RemoveDoctorPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           statusMsg_;

    ScrollableList     list_;
    int  doctorIds_[100];
    int  doctorCount_;

    Button  btnRemove_;
    Button  btnBack_;
    Screen  next_;

    void loadDoctors();
    void doRemove();
};

// ------------------------------------------------------------
// ViewAllPatientsPanel  →  Screen::ViewAllPatients
// Columns: ID | Name | Age | Gender | Contact | Balance | Unpaid Bills
// ------------------------------------------------------------
class ViewAllPatientsPanel : public UIPanel {
public:
    ViewAllPatientsPanel(GUI* gui, const Theme& theme);
    ~ViewAllPatientsPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    ScrollableList     list_;
    Button             btnBack_;
    Screen             next_;

    void loadPatients();
};

// ------------------------------------------------------------
// ViewAllDoctorsPanel  →  Screen::ViewAllDoctors
// ------------------------------------------------------------
class ViewAllDoctorsPanel : public UIPanel {
public:
    ViewAllDoctorsPanel(GUI* gui, const Theme& theme);
    ~ViewAllDoctorsPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    ScrollableList     list_;
    Button             btnBack_;
    Screen             next_;

    void loadDoctors();
};

// ------------------------------------------------------------
// ViewAllAppointmentsPanel  →  Screen::ViewAllAppointments
// Sorted date descending.
// ------------------------------------------------------------
class ViewAllAppointmentsPanel : public UIPanel {
public:
    ViewAllAppointmentsPanel(GUI* gui, const Theme& theme);
    ~ViewAllAppointmentsPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    ScrollableList     list_;
    Button             btnBack_;
    Screen             next_;

    void loadAppointments();
};

// ------------------------------------------------------------
// ViewUnpaidBillsPanel  →  Screen::ViewUnpaidBills
// Shows [OVERDUE] flag for bills > 7 days old.
// ------------------------------------------------------------
class ViewUnpaidBillsPanel : public UIPanel {
public:
    ViewUnpaidBillsPanel(GUI* gui, const Theme& theme);
    ~ViewUnpaidBillsPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    ScrollableList     list_;
    Button             btnBack_;
    Screen             next_;

    void loadUnpaidBills();
};

// ------------------------------------------------------------
// DischargePatientPanel  →  Screen::DischargePatient
// ------------------------------------------------------------
class DischargePatientPanel : public UIPanel {
public:
    DischargePatientPanel(GUI* gui, const Theme& theme);
    ~DischargePatientPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    sf::Text           statusMsg_;

    ScrollableList     list_;
    int  patientIds_[100];
    int  patientCount_;

    Button  btnDischarge_;
    Button  btnBack_;
    Screen  next_;

    void loadPatients();
    void doDischarge();
};

// ------------------------------------------------------------
// SecurityLogPanel  →  Screen::SecurityLog
// ------------------------------------------------------------
class SecurityLogPanel : public UIPanel {
public:
    SecurityLogPanel(GUI* gui, const Theme& theme);
    ~SecurityLogPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    ScrollableList     list_;
    Button             btnBack_;
    Screen             next_;

    void loadLog();
};

// ------------------------------------------------------------
// DailyReportPanel  →  Screen::DailyReport
// Derived entirely from in-memory stores (no file written).
// ------------------------------------------------------------
class DailyReportPanel : public UIPanel {
public:
    DailyReportPanel(GUI* gui, const Theme& theme);
    ~DailyReportPanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           heading_;
    ScrollableList     list_;
    Button             btnBack_;
    Screen             next_;

    void buildReport();
};

// ------------------------------------------------------------
// MessagePanel  →  Screen::Message
// Displays a single line of text (success or error) and
// a "Back" button that returns to the stored returnTo screen.
// ------------------------------------------------------------
class MessagePanel : public UIPanel {
public:
    MessagePanel(GUI* gui, const Theme& theme);
    ~MessagePanel();

    void handleEvent(const sf::Event&, sf::Vector2f, bool, bool) override;
    void update(float dt) override;
    void draw(sf::RenderTarget&) const override;
    Screen nextScreen() const override;
    void onEnter() override;

    // Called by GUI::showMessage() before switching to this panel.
    void setMessage(const char* message, Screen returnTo, bool isError);

private:
    GUI* gui_;
    const Theme* theme_;

    sf::RectangleShape panel_;
    sf::Text           msgText_;
    Button             btnOk_;

    char   message_[300];
    Screen returnTo_;
    bool   isError_;
    Screen next_;
};

#endif // GUI_H