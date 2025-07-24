class SCR_AICarpoolActivity : SCR_AIActivityBase
{
	protected static const string MOVEMENT_TYPE_PORT = "MovementType";
	
	ref SCR_BTParamAssignable<vector> m_vPosition = new SCR_BTParamAssignable<vector>(SCR_AIActionTask.POSITION_PORT);
	ref SCR_BTParam<IEntity> m_Entity = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<bool> m_bUseVehicles = new SCR_BTParam<bool>(SCR_AIActionTask.USE_VEHICLES_PORT);
	ref SCR_BTParam<EMovementType> m_eMovementType = new SCR_BTParam<EMovementType>(MOVEMENT_TYPE_PORT);
	
	//------------------------------------------------------------------------------------------------
	void InitParameters(vector position, IEntity entity, EMovementType movementType, bool useVehicles, float priorityLevel)
	{
		m_vPosition.Init(this, position);
		m_vPosition.m_AssignedOut = (position != vector.Zero);
		m_Entity.Init(this, entity);
		m_bUseVehicles.Init(this, useVehicles);
		m_eMovementType.Init(this, movementType);
		m_fPriorityLevel.Init(this, priorityLevel);	
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AICarpoolActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, vector pos, IEntity ent, EMovementType movementType = EMovementType.RUN, bool useVehicles = true, float priority = PRIORITY_ACTIVITY_MOVE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(pos, ent, movementType, useVehicles, priorityLevel);
		m_sBehaviorTree = "{2DFCB23E2747F3EA}AI/BehaviorTrees/Chimera/Group/ActivityMove_CLINTON.bt";
		SetPriority(priority);
	}
	
	//------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		if (m_Utility.HasActionOfType(SCR_AIHealActivity))
			return 0;
		
		return GetPriority();
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_vPosition.m_Value.ToString();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		if (m_bUseVehicles.m_Value)
		{
			SCR_AIGroup group = SCR_AIGroup.Cast(m_Utility.GetAIAgent());
			if (!group)
				return;
			group.ReleaseCompartments();
		}
		SendCancelMessagesToAllAgents();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		if (m_bUseVehicles.m_Value)
		{
			SCR_AIGroup group = SCR_AIGroup.Cast(m_Utility.GetAIAgent());
			if (!group)
				return;
			group.ReleaseCompartments();
		}
		SendCancelMessagesToAllAgents();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		SendCancelMessagesToAllAgents();
	}
	/*
	protected ref SCR_BTParam<AIWaypoint> m_Waypoint = new SCR_BTParam<AIWaypoint>(SCR_AIActionTask.WAYPOINT_PORT);
	const static float PRIORITY_ACTIVITY_LOOT					= 80;

	protected ref array<IEntity> m_aItemsToLoot;	// stores recently found items
	protected int m_iItemsTakenIndex; 				// remebers which batch was already requested

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] relatedWaypoint
	//! \param[in] priorityLevel
	void InitParameters(AIWaypoint relatedWaypoint, float priorityLevel)
	{
		m_Waypoint.Init(this, relatedWaypoint);
		m_fPriorityLevel.Init(this, priorityLevel);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] utility
	//! \param[in] relatedWaypoint
	//! \param[in] priority
	//! \param[in] priorityLevel
	void SCR_AILootActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, float priority = PRIORITY_ACTIVITY_LOOT, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityMove.bt";
		SetPriority(priority);
		InitParameters(relatedWaypoint, priorityLevel);
		m_aItemsToLoot = {};

		AIWaypoint wp = relatedWaypoint;
		if (wp)
			m_Waypoint.m_Value = SCR_CarpoolWaypoint.Cast(wp);
	}

	//------------------------------------------------------------------------------------------------
	//! Search area for items to loot and saves result to internal array, use GetLootForAgent to get the items
	//! \param[in] origin the loot area centre
	//! \param[in] radius the loot area radius from origin
	//! \return true if items to loot were found, false otherwise
	bool SetLoot(vector origin, float radius)
	{
		m_aItemsToLoot.Clear();
		m_iItemsTakenIndex = 0;
		GetGame().GetWorld().QueryEntitiesBySphere(origin, radius, CheckItem, FilterEntities, queryFlags: EQueryEntitiesFlags.DYNAMIC);
		return !m_aItemsToLoot.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] itemsToLoot
	void GetLootForAgent(out array<IEntity> itemsToLoot)
	{
		itemsToLoot = {};
		int numAgents = m_Utility.m_Owner.GetAgentsCount();
		if (numAgents < 1)
			return;

		int numItemsTaken = m_iItemsTakenIndex;
		float itemsForAgent = Math.Ceil(m_aItemsToLoot.Count() / numAgents);
		for (int index = numItemsTaken; index < itemsForAgent + numItemsTaken; index++)
		{
			if (!m_aItemsToLoot.IsIndexValid(index))
				break;

			itemsToLoot.Insert(m_aItemsToLoot[index]);
			m_iItemsTakenIndex++;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool FilterEntities(IEntity ent)
	{
		MagazineComponent mag = MagazineComponent.Cast(ent.FindComponent(MagazineComponent));
		if (mag)
			return true;

		WeaponComponent weapon = WeaponComponent.Cast(ent.FindComponent(WeaponComponent));
		if (weapon)
			return true;

		return false; // ignore this entity
	}

	//------------------------------------------------------------------------------------------------
	// check if it is not already part of a character's inventory
	protected bool CheckItem(IEntity ent)
	{
		InventoryItemComponent inventoryComp = InventoryItemComponent.Cast(ent.FindComponent(InventoryItemComponent));
		if (inventoryComp)
		{
			InventoryStorageSlot storageSlot = inventoryComp.GetParentSlot();
			if (!storageSlot)
				m_aItemsToLoot.Insert(ent);
		}

		return true; // searching all items
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SendCancelMessagesToAllAgents();
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		SendCancelMessagesToAllAgents();
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		SendCancelMessagesToAllAgents();
	}
	*/
}

class SCR_AIGetCarpoolActivityParameters : SCR_AIGetActionParameters
{
	
	static ref TStringArray s_aVarsOut = (new SCR_AICarpoolActivity(null, null, vector.Zero, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	static override bool VisibleInPalette() { return true; }
}

class SCR_AISetCarpoolActivityParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AICarpoolActivity(null, null, vector.Zero, null)).GetPortNames();	
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	static override bool VisibleInPalette() { return true; }
}
