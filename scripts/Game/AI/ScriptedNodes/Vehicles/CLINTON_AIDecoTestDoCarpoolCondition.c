class CLINTON_AIDecoTestDoCarpoolCondition : DecoratorScripted
{
	static const string POSITION_PORT = "Position";
	[Attribute("200.0", UIWidgets.EditBox, "Distance to decide to drive", "")]
	float m_distanceThreshold;
	private ref array<IEntity> m_aVics;
	private float radius;
	
	// stolen
	private ref array<BaseCompartmentSlot> m_CompartmentSlots = {};
	protected BaseCompartmentSlot m_Compartment;
	protected ECompartmentType m_CompartmentType;
	protected IEntity m_VehicleToTestForCompartments;
	
	protected override bool TestFunction(AIAgent owner)
	{
		vector movePosition;
		GetVariableIn(POSITION_PORT, movePosition);
		
		if (!movePosition) return false;
		
		if (!SCR_ChimeraAIAgent.Cast(owner)) return false;
		
		vector playerPosition = owner.GetOrigin();
		
		if(vector.Distance(movePosition,playerPosition) < 20.0) return false;
		
		return true;
	}
	
	//if youre gay say what?-----------------------------------------------------------------------------------------------------
	protected bool CheckItem(IEntity ent)
	{  
		m_aVics.Insert(ent);
		return true; // continue searching
	}
	
	//Also Copied------------------------------------------------------------------------------------------------
	bool FilterEntities(IEntity ent) 
	{
		if (ent.FindComponent(BaseCompartmentManagerComponent) &&
		    (Turret.Cast(ent) || SCR_AIVehicleUsability.VehicleCanMove(ent)) &&
		    !SCR_AIVehicleUsability.VehicleIsOnFire(ent)
		)	return true;
		
		return false;
	}
	
	
	//Copied from SCR_AIFindAvailableVehicle------------------------------------------------------------------------------------------------
	bool HasNoAvailableCompartment(IEntity ent) 
	{
		BaseCompartmentManagerComponent compComp = BaseCompartmentManagerComponent.Cast(ent.FindComponent(BaseCompartmentManagerComponent));
		if (!compComp)
			return true;
		
		compComp.GetCompartments(m_CompartmentSlots);
		if (m_CompartmentSlots.IsEmpty())
			return true;
		
		SCR_AIVehicleUsageComponent vehicleUsage = SCR_AIVehicleUsageComponent.Cast(ent.FindComponent(SCR_AIVehicleUsageComponent));
		if (!vehicleUsage)
			return true;
		bool canBePiloted = vehicleUsage.CanBePiloted();
		BaseCompartmentSlot pilotCompartment, turretCompartment, cargoCompartment;
		
		foreach (BaseCompartmentSlot slot : m_CompartmentSlots)
		{
			if (slot.IsOccupied() || !slot.IsCompartmentAccessible() || slot.IsReserved())
				continue;
			if (PilotCompartmentSlot.Cast(slot) && canBePiloted) //exclude helicopter pilot slots for now
				pilotCompartment = slot;
			else if (TurretCompartmentSlot.Cast(slot))
				turretCompartment = slot;
			else if (CargoCompartmentSlot.Cast(slot))
				cargoCompartment = slot;
		}
		// going through priorities: pilot > turret > cargo
		if (pilotCompartment)
		{
			m_CompartmentType = ECompartmentType.PILOT;
			m_aVics.Insert(ent);
			// m_VehicleToTestForCompartments = ent;
			m_Compartment = pilotCompartment;
			// return false;
		}
		
		if (turretCompartment)
		{
			m_CompartmentType = ECompartmentType.TURRET;
			m_aVics.Insert(ent);
			// m_VehicleToTestForCompartments = ent;
			m_Compartment = turretCompartment;
			// return false;
		}
		
		if (cargoCompartment)
		{
			m_CompartmentType = ECompartmentType.CARGO;
			m_aVics.Insert(ent);
			// m_VehicleToTestForCompartments = ent;
			m_Compartment = cargoCompartment;
			// return false;
		}
		return true; //continue search
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	//-----------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Checks time since target was detected and compares it with threshold. Returns true when time is below the threshold.";
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		POSITION_PORT
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
}