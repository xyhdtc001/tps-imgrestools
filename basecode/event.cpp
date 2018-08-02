#include "basecode/event.h"
#include "basecode/BaseType.h"

static struct ModifierBitMap
{
	uint32 grendelMask;
	uint32 torqueMask;
} _ModifierBitMap[] =
{
	{ IM_LSHIFT, SI_LSHIFT   },
	{ IM_RSHIFT, SI_RSHIFT   },
	{ IM_LALT,   SI_LALT     },
	{ IM_RALT,   SI_RALT     },
	{ IM_LCTRL,  SI_LCTRL    },
	{ IM_RCTRL,  SI_RCTRL    },
};

static int32 _ModifierBitMapCount = sizeof(_ModifierBitMap) / sizeof(ModifierBitMap);

InputModifiers convertModifierBits(const uint32 in)
{
	uint32 out=0;
	for(int32 i=0; i<_ModifierBitMapCount; i++)
	{
		if(in & _ModifierBitMap[i].grendelMask)
			out |= _ModifierBitMap[i].torqueMask;
	}
	return (InputModifiers)out;
}
