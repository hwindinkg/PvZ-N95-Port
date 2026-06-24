#ifndef __ATTACHMENT_H__
#define __ATTACHMENT_H__

#include <e32def.h>
#include "ConstEnums.h"
#include "Reanimator.h"
#include "TodParticle.h"
#include "../engine/SexyMatrix.h"

struct Attachment
{
    int mAttachmentType;
    ReanimationID mReanimID;
    ParticleSystemID mParticleID;
    float mX, mY;

    Attachment() : mAttachmentType(-1), mReanimID(REANIMATIONID_NULL), mParticleID(PARTICLESYSTEMID_NULL), mX(0), mY(0) {}
};

// AttachEffect -- returned by AttachReanim, holds transform offset
struct AttachEffect
{
    class Sexy::SexyMatrix3 mOffset;
    AttachEffect() { mOffset.LoadIdentity(); }
};

// Inline stubs for attachment functions used by Zombie.cpp
inline AttachEffect* AttachReanim(AttachmentID& attachmentID, Reanimation* reanim, float offsetX, float offsetY)
{
    (void)attachmentID; (void)reanim; (void)offsetX; (void)offsetY;
    static AttachEffect s_dummy;
    return &s_dummy;
}

inline void AttachmentDetach(AttachmentID attachmentID)
{
    (void)attachmentID;
}

inline void AttachmentDetachCrossFadeParticleType(AttachmentID attachmentID, ParticleEffect effect, void* unknown)
{
    (void)attachmentID; (void)effect; (void)unknown;
}

inline void AttachmentUpdateAndMove(AttachmentID attachmentID, float x, float y)
{
    (void)attachmentID; (void)x; (void)y;
}

inline void AttachmentDie(AttachmentID attachmentID)
{
    (void)attachmentID;
}

inline void AttachParticle(AttachmentID& attachmentID, class TodParticleSystem* particle, float offsetX, float offsetY)
{
    (void)attachmentID; (void)particle; (void)offsetX; (void)offsetY;
}

inline void AttachmentDraw(AttachmentID attachmentID, class Sexy::Graphics* g, bool a)
{
    (void)attachmentID; (void)g; (void)a;
}

inline void AttachmentCrossFade(AttachmentID attachmentID, const char* fadeOutName)
{
    (void)attachmentID; (void)fadeOutName;
}

inline void AttachmentOverrideColor(AttachmentID attachmentID, const Sexy::Color& theColor)
{
    (void)attachmentID; (void)theColor;
}

inline void AttachmentOverrideScale(AttachmentID attachmentID, float theScale)
{
    (void)attachmentID; (void)theScale;
}

#endif // __ATTACHMENT_H__
