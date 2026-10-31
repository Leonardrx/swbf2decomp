#pragma once

#include <stddef.h>
#include <string.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "GameSound.h"
#include "SoldierAnimationBank.h"

class PblConfig;
class PblMatrix;
class WeaponMelee;
class ZephyrSkeletonShared;

class Combo
{
public:
    class DamageSample;
    class Deflect;
    class Condition;
    class Transition;
    class State;

    class Condition
    {
    public:

        enum BUTTON
        {
            BUTTON_NONE = 0,
            BUTTON_FIRE = 1,
            BUTTON_FIRE_SECONDARY = 2,
            BUTTON_JUMP = 3,
            BUTTON_SPRINT = 4,
            BUTTON_CROUCH = 5,
            BUTTON_RELOAD = 6,
            BUTTON_FIRE_BOTH = 7,
            NUM_VIRTUAL_BUTTONS = 8,
            NUM_BUTTONS = 7
        };

        BUTTON GetButton() const { return (BUTTON)m_uiButton; }

        bool TestSoldierState(int iState, float fTime) const;

        bool TestEnergy(float fEnergy) const;

        bool TestInputThrust(float fThrust, float fStrafe) const;

        bool TestVelocity(float fX, float fY, float fZ) const;

        float GetInputMagRef() const { return m_uiInputMagRef * INPUT_MAG_SCALE; }
        float GetInputDirMinRad() const { return m_iInputDirMin * INPUT_DIR_SCALE_RAD; }
        float GetInputDirMaxRad() const { return m_iInputDirMax * INPUT_DIR_SCALE_RAD; }
        float GetVelocityXZDirMinRad() const { return m_iVelocityXZDirMin * VELOCITYXZ_DIR_SCALE_RAD; }
        float GetVelocityXZDirMaxRad() const { return m_iVelocityXZDirMax * VELOCITYXZ_DIR_SCALE_RAD; }
        float GetVelocityYRef() const
        {
            float fRef = (float)m_uiVelocityYRef;
            if (m_bVelocityYSigned)
            {
                return fRef * VELOCITYY_VAL_SCALE + VELOCITYY_VAL_OFFSET;
            }
            return fRef * VELOCITYY_MAG_SCALE;
        }

        void SetTimeStart(float fTime, bool bFromEnd);
        void SetTimeEnd(float fTime, bool bFromEnd, bool b);
        void SetInputMag(float fMag, bool bLess);

        void SetBreak(float fTime);
        void SetSoldierStateTime(float fTime, bool bLess);
        void SetEnergy(float fEnergy, bool bLess);
        void SetInputDir(float fMin, float fMax);
        // FUNCTION: SWBF2 0x00470bd0
        void SetVelocityY(float fVelocity, bool bSigned, bool bLess)
        {
            int iVelocity = (int)(bSigned ? (fVelocity - VELOCITYY_VAL_OFFSET) * VELOCITYY_VAL_SCALE_INV
                                          : fVelocity * VELOCITYY_MAG_SCALE_INV);
            if (iVelocity < 0)
            {
                iVelocity = 0;
            }
            else if (iVelocity > 63)
            {
                iVelocity = 63;
            }
            m_uiVelocityYRef = iVelocity;
            m_bVelocityYSigned = bSigned;
            m_bVelocityYLess = bLess;
        }
        void SetVelocityXZMag(float fMag, bool bLess);
        void SetVelocityXZDir(float fMin, float fMax);

        void SetSoldierStates(unsigned int uiStates) { m_uiSoldierStates = uiStates; }
        void SetButton(unsigned int eButton, unsigned int eAction)
        {
            m_uiButton = eButton;
            m_uiButtonAction = eAction;
        }
        void SetInputButtonsPressed(unsigned int uiButtons) { m_uiInputButtonsPressed = uiButtons; }
        void SetInputButtonsReleased(unsigned int uiButtons) { m_uiInputButtonsReleased = uiButtons; }
        bool IsBreak() const { return m_bBreak != 0; }

        void SetBreak(bool bBreak)
        {
            m_uiBreakTime = 0;
            m_bBreak = bBreak;
        }

    private:
        friend class Transition;
        friend class State;

        void _Init()
        {
            memset(m_auiPackedData, 0, sizeof(m_auiPackedData));
            m_bTimeEndFromEnd = 1;
        }

        void _ReadConfig(PblConfig& config, const char* pszComboName, const char* pszStateName);

    public:

        static MemoryPool sMemoryPool;

        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

        static const float TIME_SCALE_INV;
        static const float TIME_SCALE;
        static const float ENERGY_SCALE_INV;
        static const float ENERGY_SCALE;
        static const float INPUT_MAG_SCALE_INV;
        static const float INPUT_MAG_SCALE;
        static const float INPUT_DIR_SCALE_RAD;
        static const float VELOCITYY_MAG_SCALE_INV;
        static const float VELOCITYY_MAG_SCALE;
        static const float VELOCITYY_VAL_SCALE_INV;
        static const float VELOCITYY_VAL_SCALE;
        static const float VELOCITYY_VAL_OFFSET;
        static const float VELOCITYXZ_MAG_SCALE_INV;
        static const float VELOCITYXZ_MAG_SCALE;
        static const float VELOCITYXZ_DIR_SCALE_RAD;

        union
        {
            struct
            {
                unsigned int m_uiBreakTime : 9;
                unsigned int m_bBreak : 1;
                int m_iTimeStart : 10;
                unsigned int m_bTimeStartFromEnd : 1;
                int m_iTimeEnd : 10;
                unsigned int m_bTimeEndFromEnd : 1;

                unsigned int m_uiSoldierStates : 8;
                unsigned int m_uiSoldierStateTimeRef : 11;
                unsigned int m_bSoldierStateTimeLess : 1;
                unsigned int m_uiEnergyRef : 10;
                unsigned int m_bEnergyLess : 1;
                unsigned int m_bSyncIfOvertime : 1;

                unsigned int m_uiButtonAction : 4;
                unsigned int m_uiButton : 3;
                unsigned int z_uiReserved2 : 5;
                unsigned int m_uiInputButtonsPressed : 6;
                unsigned int m_uiInputButtonsReleased : 6;
                unsigned int m_uiVelocityYRef : 6;
                unsigned int m_bVelocityYLess : 1;
                unsigned int m_bVelocityYSigned : 1;

                unsigned int m_uiInputMagRef : 7;
                unsigned int m_bInputMagLess : 1;
                unsigned int z_uiReserved3 : 6;
                int m_iInputDirMin : 9;
                int m_iInputDirMax : 9;

                unsigned int m_uiVelocityXZMagRef : 11;
                unsigned int m_bVelocityXZMagLess : 1;
                unsigned int z_uiReserved4 : 2;
                int m_iVelocityXZDirMin : 9;
                int m_iVelocityXZDirMax : 9;
            };
            struct
            {
                unsigned int zz_0 : 32;
                unsigned int zz_1a : 8;
                unsigned int m_uiSoldierStateTimeCnd : 12;
                unsigned int m_uiEnergyCnd : 11;
                unsigned int zz_1b : 1;
                unsigned int zz_2 : 12;
                unsigned int m_uiInputButtonCnd : 12;
                unsigned int m_uiVelocityYCnd : 8;
                unsigned int m_uiInputMagCnd : 8;
                unsigned int zz_3 : 24;
                unsigned int m_uiVelocityXZMagCnd : 12;
                unsigned int zz_4 : 20;
            };
            unsigned int m_auiPackedData[5];
        };
    };

    class Transition
    {
    public:

        unsigned int GetNumConditions() const { return m_uiNumConditions; }
        const Condition* GetCondition(unsigned int iCondition) const { return &m_aCondition[iCondition]; }

        Condition* AddCondition();

        void SetEnergyCost(float fCost)
        {
            int iCost = (int)(fCost * ENERGY_SCALE_INV);
            if (iCost < 0)
            {
                iCost = 0;
            }
            else if (iCost > 1023)
            {
                iCost = 1023;
            }
            m_uiEnergyCost = iCost;
        }

    private:
        friend class State;

        void _Init(unsigned int uiTargetState)
        {
            memset(this, 0, sizeof(*this));
            m_uiTargetState = uiTargetState;
        }

        void _ReadConfig(PblConfig& config, const char* pszComboName, const char* pszStateName);

    public:

        static MemoryPool sMemoryPool;

        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

        static const float ENERGY_SCALE_INV;

        unsigned int m_uiTargetState;
        Condition* m_aCondition;
        unsigned int m_uiNumConditions : 6;
        unsigned int m_uiEnergyCost : 10;
        unsigned int z_uiReserved0 : 16;
    };

    class DamageSample
    {
    public:
        static MemoryPool sMemoryPool;
        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }
        static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

        float fTime;
        PblVector3 vPos;
        PblVector3 vDir;
    };


    class Attack
    {
    public:
        unsigned int GetAttackId() const { return m_uiAttackId; }
        unsigned int GetDamageEdge() const { return m_uiDamageEdge; }
        bool IsDamageTimeStartFromAnim() const { return m_bDamageTimeStartFromAnim != 0; }
        bool IsDamageTimeEndFromAnim() const { return m_bDamageTimeStartFromAnim != 0; }
        float GetDamageTimeStart() const { return m_uiDamageTimeStart * 0.01f; }
        float GetDamageTimeEnd() const { return m_uiDamageTimeEnd * 0.01f; }
        bool IsDamageLengthFromEdge() const { return m_bDamageLengthFromEdge != 0; }
        bool IsDamageWidthFromEdge() const { return m_bDamageWidthFromEdge != 0; }
        float GetDamageLength() const { return m_uiDamageLength * 0.01f; }
        float GetDamageWidth() const { return m_uiDamageWidth * 0.01f; }
        float GetDamage() const { return m_uiDamage * 1.0f; }
        bool IsPush() const { return m_uiSpecialType == 0; }
        float GetPushStrength() const { return m_uiSpecial * 0.1f; }
        unsigned int GetNumDamageSamples() const { return m_uiNumDamageSamples; }
        const DamageSample* GetDamageSample(unsigned int iSample) const { return &m_aDamageSample[iSample]; }

        void AddDamageSample(float fTime, const PblVector3& vBegin, const PblVector3& vEnd);
        bool _ResolveDamageData(const Combo* pCombo, const State* pState,
                                unsigned int uiAttach, const PblVector3& vBegin,
                                const PblVector3& vEnd);

        void _Init();

        // FUNCTION: SWBF2 0x00470d10
        void SetDamageTimeStart(float fTime, bool bFromAnim)
        {
            int iTime = (int)(fTime * TIME_SCALE_INV);
            if (iTime < 0)
            {
                iTime = 0;
            }
            else if (iTime > 1023)
            {
                iTime = 1023;
            }
            m_uiDamageTimeStart = iTime;
            m_bDamageTimeStartFromAnim = bFromAnim;
        }
        // FUNCTION: SWBF2 0x00470d60
        void SetDamageTimeEnd(float fTime, bool bFromAnim)
        {
            int iTime = (int)(fTime * TIME_SCALE_INV);
            if (iTime < 0)
            {
                iTime = 0;
            }
            else if (iTime > 1023)
            {
                iTime = 1023;
            }
            m_uiDamageTimeEnd = iTime;
            m_bDamageTimeEndFromAnim = bFromAnim;
        }
        // FUNCTION: SWBF2 0x00470db0
        void SetDamageLength(float fLength, bool bFromEdge)
        {
            int iLength = (int)(fLength * LENGTH_SCALE_INV);
            if (iLength < 0)
            {
                iLength = 0;
            }
            else if (iLength > 1023)
            {
                iLength = 1023;
            }
            m_uiDamageLength = iLength;
            m_bDamageLengthFromEdge = bFromEdge;
        }
        void SetDamage(float fDamage);

        void SetDamage() { m_uiDamage = 0xfff; }
        void SetDamageEdge(unsigned int uiEdge)
        {
            int iEdge = (int)uiEdge;
            if (iEdge < 0)
            {
                iEdge = 0;
            }
            else if (iEdge > 15)
            {
                iEdge = 15;
            }
            m_uiDamageEdge = iEdge;
        }
        void SetDamageWidth(float fWidth, bool bFromEdge)
        {
            int iWidth = (int)(fWidth * WIDTH_SCALE_INV);
            if (iWidth < 0)
            {
                iWidth = 0;
            }
            else if (iWidth > 255)
            {
                iWidth = 255;
            }
            m_uiDamageWidth = iWidth;
            m_bDamageWidthFromEdge = bFromEdge;
        }

        void SetPushStrength(float fStrength)
        {
            int iStrength = (int)(fStrength * PUSH_SCALE_INV);
            if (iStrength < 0)
            {
                iStrength = 0;
            }
            else if (iStrength > 255)
            {
                iStrength = 255;
            }
            m_uiSpecialType = 0;
            m_uiSpecial = iStrength;
        }
        void SetAttackId(unsigned int uiAttackId) { m_uiAttackId = uiAttackId; }

    private:
        friend class State;

        void _ReadConfig(PblConfig& config, const char* pszStateName);

    public:
        static MemoryPool sMemoryPool;

        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

        static const float WIDTH_SCALE_INV;
        static const float PUSH_SCALE_INV;
        static const float TIME_SCALE_INV;
        static const float LENGTH_SCALE_INV;

        union
        {
            struct
            {
                unsigned int m_uiNumDamageSamples : 8;
                unsigned int m_bDamageTimeStartFromAnim : 1;
                unsigned int m_bDamageTimeEndFromAnim : 1;
                unsigned int m_uiDamageTimeStart : 10;
                unsigned int m_uiDamageTimeEnd : 10;
                unsigned int m_uiSpecialType : 2;

                unsigned int m_uiDamage : 12;
                unsigned int m_uiAttackId : 20;

                unsigned int m_uiDamageEdge : 4;
                unsigned int m_bDamageLengthFromEdge : 1;
                unsigned int m_bDamageWidthFromEdge : 1;
                unsigned int m_uiDamageLength : 10;
                unsigned int m_uiDamageWidth : 8;
                unsigned int m_uiSpecial : 8;
            };
            unsigned int m_auiPackedData[3];
        };
        DamageSample* m_aDamageSample;
    };

    class State
    {
    public:

        unsigned char GetAnimIndex() const { return (unsigned char)m_uiAnimIndex; }
        void SetAnimIndex(unsigned char uiAnimIndex) { m_uiAnimIndex = uiAnimIndex; }
        const Deflect* GetDeflect() const { return m_pDeflect; }
        unsigned int GetNumTransitions() const { return m_uiNumTransitions; }
        const Transition* GetTransition(unsigned int iTransition) const { return &m_aTransition[iTransition]; }
        unsigned int GetNumAttacks() const { return m_uiNumAttacks; }
        Attack* GetAttack(unsigned int iAttack) { return &m_aAttack[iAttack]; }
        const Attack* GetAttack(unsigned int iAttack) const { return &m_aAttack[iAttack]; }
        float GetDuration() const { return m_uiDuration * 0.01f; }
        bool IsAnimatedMove() const { return m_bAnimatedMove != 0; }
        bool IsDuration() const { return m_uiDuration != DURATION_INVALID; }
        bool IsVelocityZFromAnim() const { return m_bVelocityZFromAnim && m_bAnimatedMove; }
        // FUNCTION: SWBF2 0x00470ea0
        bool IsVelocityXFromAnim() const { return m_bVelocityXFromAnim && m_bAnimatedMove; }
        float GetVelocityZ() const { return m_iVelocityZ * 0.02f; }
        float GetVelocityX() const { return m_iVelocityX * 0.02f; }
        float GetVelocityFromThrust() const { return m_uiVelocityFromThrust * 0.1f; }
        float GetVelocityFromStrafe() const { return m_uiVelocityFromStrafe * 0.1f; }
        bool IsAlignedToThrust() const { return m_bAlignToThrust != 0; }

        // FUNCTION: SWBF2 0x00470ec0
        void SetDuration(float fDuration)
        {
            if (fDuration < 0.0f)
            {
                m_uiDuration = DURATION_INVALID;
                return;
            }
            int iDuration = (int)(fDuration * TIME_SCALE_INV);
            if (iDuration < 0)
            {
                iDuration = 0;
            }
            else if (iDuration > DURATION_MAX)
            {
                iDuration = DURATION_MAX;
            }
            m_uiDuration = iDuration;
        }

        void SetSound(const char* pName, float fTime);

        // FUNCTION: SWBF2 0x00470f70
        void SetVelocityZ(float fVelocity, bool bFromAnim)
        {
            int iVelocity = (int)(fVelocity * VELOCITY_SCALE_INV);
            if (iVelocity < VELOCITY_MIN)
            {
                iVelocity = VELOCITY_MIN;
            }
            else if (iVelocity > VELOCITY_MAX)
            {
                iVelocity = VELOCITY_MAX;
            }
            m_iVelocityZ = iVelocity;
            m_bVelocityZFromAnim = bFromAnim;
        }

        // FUNCTION: SWBF2 0x00470fc0
        void SetVelocityX(float fVelocity, bool bFromAnim)
        {
            int iVelocity = (int)(fVelocity * VELOCITY_SCALE_INV);
            if (iVelocity < VELOCITY_MIN)
            {
                iVelocity = VELOCITY_MIN;
            }
            else if (iVelocity > VELOCITY_MAX)
            {
                iVelocity = VELOCITY_MAX;
            }
            m_iVelocityX = iVelocity;
            m_bVelocityXFromAnim = bFromAnim;
        }

        // FUNCTION: SWBF2 0x00471010
        void SetInputLock(unsigned int uiInputLock, float fDuration)
        {
            m_uiInputLock = uiInputLock;
            int iDuration = (int)(fDuration * TIME_SCALE_INV);
            if (iDuration < 0)
            {
                iDuration = 0;
            }
            else if (iDuration > INPUTLOCK_DURATION_MAX)
            {
                iDuration = INPUTLOCK_DURATION_MAX;
            }
            m_uiInputLockDuration = iDuration;
        }

        // FUNCTION: SWBF2 0x00471060
        void SetEnergyRestoreRate(float fRate, bool b)
        {
            int iRate = (int)(fRate * ENERGY_RATE_SCALE_INV);
            if (iRate < ENERGY_RATE_MIN)
            {
                iRate = ENERGY_RATE_MIN;
            }
            else if (iRate > ENERGY_RATE_MAX)
            {
                iRate = ENERGY_RATE_MAX;
            }
            m_iEnergyRestoreRate = iRate;
            m_bEnergyRestoreRateOverride = !b;
        }

        // FUNCTION: SWBF2 0x004710c0
        void SetThrustFactor(float fFactor)
        {
            int iFactor = (int)(fFactor * INPUT_SCALE_INV);
            if (iFactor < 0)
            {
                iFactor = 0;
            }
            else if (iFactor > INPUT_MAX)
            {
                iFactor = INPUT_MAX;
            }
            m_uiThrustFactor = iFactor;
        }

        // FUNCTION: SWBF2 0x004710f0
        void SetStrafeFactor(float fFactor)
        {
            int iFactor = (int)(fFactor * INPUT_SCALE_INV);
            if (iFactor < 0)
            {
                iFactor = 0;
            }
            else if (iFactor > INPUT_MAX)
            {
                iFactor = INPUT_MAX;
            }
            m_uiStrafeFactor = iFactor;
        }

        // FUNCTION: SWBF2 0x00471120
        void SetPitchFactor(float fFactor)
        {
            int iFactor = (int)(fFactor * INPUT_SCALE_INV);
            if (iFactor < 0)
            {
                iFactor = 0;
            }
            else if (iFactor > INPUT_MAX)
            {
                iFactor = INPUT_MAX;
            }
            m_uiPitchFactor = iFactor;
        }

        void SetGravity(float fGravity);

        // FUNCTION: SWBF2 0x00471180
        void SetGravityVelocityYTarget(float fVelocity, bool bZeroAtTarget)
        {
            int iVelocity = (int)(fVelocity * VELOCITYY_SCALE_INV);
            if (iVelocity < VELOCITYY_MIN)
            {
                iVelocity = VELOCITYY_MIN;
            }
            else if (iVelocity > VELOCITYY_MAX)
            {
                iVelocity = VELOCITYY_MAX;
            }
            m_iGravityVelocityYTarget = iVelocity;
            m_bGravityZeroAtTarget = bZeroAtTarget;
        }

        // FUNCTION: SWBF2 0x004711e0
        void SetVelocityYImpulse(float fVelocity, bool bZeroAtTarget)
        {
            m_iGravity = GRAVITY_IMPULSE;
            SetGravityVelocityYTarget(fVelocity, bZeroAtTarget);
        }

        Transition* AddTransition(unsigned int uiTargetState);
        Attack* AddAttack();

        const State& operator=(const State& other);
        Deflect* AddDeflect();

        Condition* AddAnimatedMoveCondition();

        void SetEnergyRestoreRate()
        {
            m_iEnergyRestoreRate = 0;
            m_bEnergyRestoreRateOverride = 0;
        }
        void SetSoldierStates(unsigned int uiStates) { m_uiSoldierStates = uiStates; }
        void SetTurnFactor(float fFactor)
        {
            int iFactor = (int)(fFactor * INPUT_SCALE_INV);
            if (iFactor < 0)
            {
                iFactor = 0;
            }
            else if (iFactor > INPUT_MAX)
            {
                iFactor = INPUT_MAX;
            }
            m_uiTurnFactor = iFactor;
        }
        void SetVelocityFromThrust(float fFactor)
        {
            int iFactor = (int)(fFactor * VELOCITYDELTA_SCALE_INV);
            if (iFactor < 0)
            {
                iFactor = 0;
            }
            else if (iFactor > 255)
            {
                iFactor = 255;
            }
            m_uiVelocityFromThrust = iFactor;
        }
        void SetVelocityFromStrafe(float fFactor)
        {
            int iFactor = (int)(fFactor * VELOCITYDELTA_SCALE_INV);
            if (iFactor < 0)
            {
                iFactor = 0;
            }
            else if (iFactor > 255)
            {
                iFactor = 255;
            }
            m_uiVelocityFromStrafe = iFactor;
        }
        void SetAnimatedMove(bool bAnimatedMove) { m_bAnimatedMove = bAnimatedMove; }
        void SetAlignedToThrust(bool bAligned, bool bInitial)
        {
            m_bAlignToThrust = bAligned;
            m_bAlignToInitialThrust = bInitial;
        }
        void SetMustShowOneFrame(bool b) { m_bMustShowOneFrame = b; }
        void SetPlayExplosionOnEnter(bool b) { m_bPlayExplosion = b; }
        void SetRestartAnimationOnEnter(bool b) { m_bRestartAnimation = b; }
        void SetLightsaberOff(bool bOff) { m_uiLightsaberOffMask = bOff ? 0xf : 0; }
        void SetLightsaberOff(unsigned int uiBlade, bool bOff)
        {
            if (uiBlade < 4)
            {
                if (bOff)
                {
                    m_uiLightsaberOffMask |= 1 << uiBlade;
                }
                else
                {
                    m_uiLightsaberOffMask &= ~(1 << uiBlade);
                }
            }
        }

    private:
        friend class Combo;

        void _Init(unsigned int uiNameHash);

        void _ReadConfig(PblConfig& config, const Combo& combo, const char* pszComboName);

        void _ReadAnimatedMoveScope(PblConfig& config, const char* pszComboName);

    public:

        static MemoryPool sMemoryPool;

        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

        static const float TIME_SCALE_INV;
        static const float ENERGY_RATE_SCALE_INV;
        static const float INPUT_SCALE_INV;
        static const float VELOCITY_SCALE_INV;
        static const float GRAVITY_SCALE_INV;
        static const float VELOCITYY_SCALE_INV;
        static const float VELOCITYDELTA_SCALE_INV;
        static const unsigned char ANIMATION_INVALID = 0xff;
        static const int DURATION_MAX = 1022;
        static const unsigned int DURATION_INVALID = 0x3ff;
        static const int INPUTLOCK_DURATION_MAX = 1023;
        static const int SOUND_TIME_MAX = 1023;
        static const int ENERGY_RATE_MIN = -512;
        static const int ENERGY_RATE_MAX = 511;
        static const int INPUT_MAX = 255;
        static const int VELOCITY_MIN = -2048;
        static const int VELOCITY_MAX = 2047;
        static const int GRAVITY_MIN = -127;
        static const int GRAVITY_MAX = 127;
        static const int GRAVITY_IMPULSE = -128;
        static const int VELOCITYY_MIN = -2047;
        static const int VELOCITYY_MAX = 2047;

        unsigned int m_uiNameHash;
        GameSound m_sound;
        Transition* m_aTransition;
        Attack* m_aAttack;
        Deflect* m_pDeflect;
        Condition* m_aAnimatedMoveCondition;
        union
        {
            struct
            {
                unsigned int m_uiAnimIndex : 8;
                unsigned int m_uiSoldierStates : 8;
                unsigned int m_uiNumTransitions : 6;
                unsigned int m_uiNumAnimatedMoveConditions : 6;
                unsigned int m_uiNumAttacks : 4;

                unsigned int m_uiDuration : 10;
                unsigned int m_uiSoundTime : 10;
                int m_iEnergyRestoreRate : 10;
                unsigned int m_bEnergyRestoreRateOverride : 1;
                unsigned int m_bMustShowOneFrame : 1;

                unsigned int m_uiThrustFactor : 8;
                unsigned int m_uiStrafeFactor : 8;
                unsigned int m_uiTurnFactor : 8;
                unsigned int m_uiPitchFactor : 8;

                unsigned int m_uiInputLock : 8;
                unsigned int m_uiInputLockDuration : 10;
                unsigned int z_uiReserved3 : 1;
                unsigned int m_bAnimatedMove : 1;
                unsigned int m_bAlignToThrust : 1;
                unsigned int m_bAlignToInitialThrust : 1;
                unsigned int m_bVelocityZFromAnim : 1;
                unsigned int m_bVelocityXFromAnim : 1;
                unsigned int m_uiVelocityFromThrust : 8;

                unsigned int m_uiVelocityFromStrafe : 8;
                int m_iVelocityZ : 12;
                int m_iVelocityX : 12;

                int m_iGravity : 8;
                int m_iGravityVelocityYTarget : 12;
                unsigned int m_bGravityZeroAtTarget : 1;
                unsigned int z_uiReserved5 : 5;
                unsigned int m_bRestartAnimation : 1;
                unsigned int m_bPlayExplosion : 1;
                unsigned int m_uiLightsaberOffMask : 4;
            };
            unsigned int m_auiPackedData[6];
        };
    };
    class Deflect
    {
    public:

        enum DEFLECT_DIRECTION
        {
            DEFLECT_FORWARD = 1,
            DEFLECT_RIGHT = 2,
            DEFLECT_BACK_RIGHT = 4,
            DEFLECT_LEFT = 8,
            DEFLECT_BACK_LEFT = 16,
            DEFLECT_BACK = 32,
            DEFLECT_ALL = 63,
            NUM_DIRECTIONS = 6
        };

        bool TestDeflect(const PblMatrix& mSoldier, const PblVector3& vFacingDir,
                         const PblVector3& vDir, const PblVector3& vCollisionPoint,
                         unsigned char uiAnimIndexPrev, unsigned char& o_uiAnimIndex) const;
        struct DEFLECT_ANIM
        {
            unsigned char uiAnimIndex;
            unsigned char uiDirections : 6;
            unsigned char uiWeapon : 2;
        };

        unsigned int GetNumDeflectAnimations() const { return m_uiNumDeflectAnimations; }
        unsigned char GetDeflectAnimIndex(unsigned int i) const { return m_aDeflectAnim[i].uiAnimIndex; }

        unsigned int AddDeflectAnimation(unsigned char uiAnimIndex, unsigned char uiDirections,
                                         unsigned char uiWeapon);

        void SetDeflectAngle(float fMin, float fMax);

        void SetEnergyCost(float fCost)
        {
            int iCost = (int)(fCost * ENERGY_SCALE_INV);
            if (iCost < 0)
            {
                iCost = 0;
            }
            else if (iCost > 1023)
            {
                iCost = 1023;
            }
            m_uiEnergyCost = iCost;
        }

    private:
        friend class State;

        void _Init()
        {
            memset(m_auiPackedData, 0, sizeof(m_auiPackedData));
        }

        void _ReadConfig(PblConfig& config, const Combo& combo, const char* pszStateName);

    public:
        static MemoryPool sMemoryPool;

        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

        static const float ENERGY_SCALE_INV;

        union
        {
            struct
            {
                unsigned int m_uiNumDeflectAnimations : 4;
                int m_iDeflectAngleMin : 9;
                int m_iDeflectAngleMax : 9;
                unsigned int m_uiEnergyCost : 10;
                DEFLECT_ANIM m_aDeflectAnim[15];
                unsigned short z_uiReserved;
            };
            unsigned int m_auiPackedData[9];
        };
    };

    static void Init();

    Combo();
    Combo(const Combo& copy) { *this = copy; }
    const Combo& operator=(const Combo& copy);
    void ReadConfig(PblConfig& config);
    static void PostLoad();
    static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }
    // SYNTHETIC: SWBF2 0x00470950
    // Combo::operator delete
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static const Combo* Find(unsigned int uiNameHash) { return s_ComboTable.Find(uiNameHash); }
    unsigned int GetNumStates() const { return m_uiNumStates; }
    State* GetState(unsigned int i) { return &m_aState[i]; }
    const State* GetState(unsigned int i) const { return &m_aState[i]; }

    static MemoryPool sMemoryPool;
    static PblHashTable<const Combo, 32> s_ComboTable;

    void InitDefault(unsigned int uiNameHash);
    void AddDefaultAttackState();
    void AddDefaultDeflect();
    State* AddState(unsigned int uiNameHash);
    unsigned char GetOffhandAnimIndex(int eState) const;
    const Combo* ResolveForWeapon(const WeaponMelee* pWeapon, const ZephyrSkeletonShared* pSkeleton) const;

    void SetOffhandAnimIndex(int eState, unsigned char uiAnimIndex);

    void SetThrowAnimIndex(unsigned char uiAnimIndex) { m_uiThrowAnimIndex = uiAnimIndex; }

private:

    static Transition* _AllocTransition();
    static Condition* _AllocCondition();
    static Attack* _AllocAttack();
    static Deflect* _AllocDeflect();

    static State* _AllocState();

    void _PostLoad();
    unsigned int _GetDeflectButton() const;

public:

    static DamageSample* _AllocDamageSample();

    static unsigned int __fastcall GetNameHashForMap(unsigned int uComboHash, SoldierAnimationBank::MAP eMap);

    unsigned int m_uiNameHash;
    int m_eCombo;
    int m_eMap;
    const ZephyrSkeletonShared* m_pSkeleton;
    State* m_aState;
    State* m_pCatchState;
    State* m_pTakeKneeState;
    unsigned char m_auiOffhandAnimIndex[4];
    unsigned int m_uiThrowAnimIndex : 8;
    unsigned int m_uiNumStates : 8;
    unsigned int m_uiDeflectButton : 8;
    unsigned int m_uiReserved0 : 8;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
