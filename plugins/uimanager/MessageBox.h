#ifndef _UI_MESSAGEBOX_H_
#define _UI_MESSAGEBOX_H_

#include "Dialog.h"

typedef struct _GtkAccelGroup GtkAccelGroup;

namespace ui
{

/**
 * A MessageBox is a specialised Dialog used for popup messages of various purpose.
 * Supported are things like Notifications, Warnings, Errors and simple Yes/No questions.
 *
 * Each messagebox is equipped with a special GTK stock icon, corresponding to its type.
 */
class MessageBox :
	public Dialog
{
protected:
	// The message text
	std::string _text;

	// The message type
	IDialog::MessageType _type;

	// Keyboard accel group used to map ENTER and ESC to buttons
	GtkAccelGroup* _accelGroup;

public:
	// Constructs a new messageBox using the given title and text
	MessageBox(std::size_t id, DialogManager& owner, 
			   const std::string& title, 
			   const std::string& text, 
			   IDialog::MessageType type);

protected:
	// Constructs the dialog (adds buttons, text and icons)
	virtual void construct();

	// Override Dialog::createButtons() to add the custom ones
	virtual GtkWidget* createButtons();

	// Creates an icon from stock (notification, warning, error)
	GtkWidget* createIcon();

	void mapKeyToButton(guint key, GtkWidget* button);

	// GTK Callbacks, additional to the ones defined in the base class
	static void onYes(GtkWidget* widget, MessageBox* self);
	static void onNo(GtkWidget* widget, MessageBox* self);
};
typedef boost::shared_ptr<MessageBox> MessageBoxPtr;

} // namespace ui

#endif /* _UI_MESSAGEBOX_H_ */
