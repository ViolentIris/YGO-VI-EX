#ifndef MENU_HANDLER_H
#define MENU_HANDLER_H

#include <irrlicht.h>
#include "replay.h"
#include "config.h"

namespace ygo {

class MenuHandler: public irr::IEventReceiver {
public:
	virtual bool OnEvent(const irr::SEvent& event);
	s32 prev_operation;
	int prev_sel;
	Replay temp_replay;
};

}

#endif //MENU_HANDLER_H
