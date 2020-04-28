#include "cbase.h"
#include "weapon_m41s.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponM41S, DT_WeaponM41S)

BEGIN_NETWORK_TABLE(CWeaponM41S, DT_WeaponM41S)
	DEFINE_NEO_BASE_WEP_NETWORK_TABLE
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponM41S)
	DEFINE_NEO_BASE_WEP_PREDICTION
END_PREDICTION_DATA()
#endif

NEO_IMPLEMENT_ACTTABLE(CWeaponM41S)

LINK_ENTITY_TO_CLASS(weapon_m41s, CWeaponM41S);

PRECACHE_WEAPON_REGISTER(weapon_m41s);

CWeaponM41S::CWeaponM41S()
{
	m_flSoonestAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0;

	m_nNumShotsFired = 0;
}

void CWeaponM41S::DryFire()
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponM41S::PrimaryAttack()
{
	if (m_iClip1 == 0)
	{
		if (!m_bFireOnEmpty)
		{
			CheckReload();
		}
		else
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = 0.2;
		}

		return;
	}

	auto owner = ToBasePlayer(GetOwner());

	if (owner)
	{
		if (!m_iClip1 && !ClientWantsAutoReload(GetOwner()))
		{
			return;
		}

		// Do nothing if we hold fire whilst semi auto
		if (!(owner->m_afButtonPressed & IN_ATTACK))
		{
			return;
		}
	}

	if ((gpGlobals->curtime - m_flLastAttackTime) > 0.5f)
	{
		m_nNumShotsFired = 0;
	}
	else
	{
		m_nNumShotsFired++;
	}

	m_flLastAttackTime = gpGlobals->curtime;

	if (owner)
	{
		owner->ViewPunchReset();
	}

	BaseClass::PrimaryAttack();

	m_flAccuracyPenalty += M41_S_ACCURACY_SHOT_PENALTY_TIME;
}

void CWeaponM41S::UpdatePenaltyTime()
{
	auto owner = ToBasePlayer(GetOwner());

	if (!owner)
	{
		return;
	}

	if (((owner->m_nButtons & IN_ATTACK) == false) &&
		(m_flSoonestAttack < gpGlobals->curtime))
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp(m_flAccuracyPenalty,
			0.0f, M41_S_ACCURACY_MAXIMUM_PENALTY_TIME);
	}
}

void CWeaponM41S::ItemPreFrame()
{
	UpdatePenaltyTime();

	BaseClass::ItemPreFrame();
}

void CWeaponM41S::ItemBusyFrame()
{
	UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

void CWeaponM41S::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if (m_bInReload)
	{
		return;
	}

	auto owner = ToBasePlayer(GetOwner());

	if (!owner)
	{
		return;
	}

	if (owner->m_nButtons & IN_ATTACK)
	{
		if (m_flSoonestAttack < gpGlobals->curtime)
		{
			if (m_iClip1 <= 0)
			{
				DryFire();

				m_flSoonestAttack = gpGlobals->curtime + M41_S_FASTEST_DRY_REFIRE_TIME;
			}
			else
			{
				m_flSoonestAttack = gpGlobals->curtime + M41_S_FASTEST_REFIRE_TIME;
			}
		}
	}
}

Activity CWeaponM41S::GetPrimaryAttackActivity()
{
	if (m_nNumShotsFired < 1)
	{
		return ACT_VM_PRIMARYATTACK;
	}

	if (m_nNumShotsFired < 2)
	{
		return ACT_VM_RECOIL1;
	}

	if (m_nNumShotsFired < 3)
	{
		return ACT_VM_RECOIL2;
	}

	return ACT_VM_RECOIL3;
}

void CWeaponM41S::AddViewKick()
{
	auto owner = ToBasePlayer(GetOwner());

	if (!owner)
	{
		return;
	}

	QAngle viewPunch;

	viewPunch.x = SharedRandomFloat("m41sx", 0.25f, 0.5f);
	viewPunch.y = SharedRandomFloat("m41sy", -0.6f, 0.6f);
	viewPunch.z = 0;

	owner->ViewPunch(viewPunch);
}
