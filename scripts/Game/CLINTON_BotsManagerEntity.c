[EntityEditorProps(category: "Tutorial/Entities", description: "Attach the System to this")]
class CLINTON_EntitySpawnerEntityClass : GenericEntityClass
{
	// See: https://community.bistudio.com/wiki/Arma_Reforger:Create_an_Entity
}
CLINTON_EntitySpawnerEntityClass g_RplEntitySpawnerClassInst;

class CLINTON_EntitySpawnerEntity : GenericEntity
{	
	protected CLINTON_EntitySpawnerEntity s_Instance;
	
	//------------------------------------------------------------------------------------------------
	void CLINTON_EntitySpawnerEntity(IEntitySource src, IEntity parent)
	{
		// if( GetGame().GetWorld().IsEditMode()) return;
		SetEventMask(EntityEvent.INIT);
	}
	override void EOnInit(IEntity owner)
	{
		// super.EOnInit(owner);
	  	RplComponent m_RplComp = RplComponent.Cast(this.FindComponent(RplComponent));
	  	if (!m_RplComp)
	    	Print("uh oh...");
		if (m_RplComp.Role() != RplRole.Authority)
			return;
		if (s_Instance)
		{
			Print("Only one instance of CLINTON_BotsManagerEntity is allowed in the world!", LogLevel.WARNING);
			//delete this;
			return;
		}
		s_Instance = this;
		CLINTON_BotsManagerSystem sysComp = CLINTON_BotsManagerSystem.Cast(this.FindComponent(CLINTON_BotsManagerSystem));
	  	if (!sysComp)
	    	Print("Attach the System (component) to this");
	}
}

[EntityEditorProps(category: "Tutorial/Entities", description: "Manage Virtual Players")]
class CLINTON_BotsManagerSystemClass : ScriptComponentClass { }

CLINTON_BotsManagerSystemClass g_RplBotsManagerSystemClassInst;

class CLINTON_BotsManagerSystem : ScriptComponent
{
    static const ResourceName s_ControllerPrefab = "{895209F582E725B7}Prefabs/RplBotsManagerController_This.et";
	//[RplProp(onRplName: "OnVirtManagerUpdated")]
	protected ref CLINTON_VirtualPlayerManager m_VirtualPlayerManager;  // Maybe the characters are not syncing? Also that should be server only?
	
	protected float m_fWaitingTime = float.INFINITY;
	protected int m_iCycleDuration = 2;				// in seconds
	protected RplComponent m_RplComp;
	
	ref RplBotsManagerSessionListener m_SessionListener = new RplBotsManagerSessionListener(this); 
    ref map<RplIdentity, RplBotsManagerController> m_Controllers = new map<RplIdentity, RplBotsManagerController>();
			
	override void OnPostInit(IEntity owner)
    {
        if (g_Game.InPlayMode())
            SetEventMask(owner, EntityEvent.INIT);
    }
    
	override void EOnInit(IEntity owner)
	{
		RplMode mode = RplSession.Mode();
        if (mode != RplMode.Client)
        {
            RplSession.RegisterCallbacks(m_SessionListener);
        }

        if (mode == RplMode.None || mode == RplMode.Listen || mode == RplMode.Client) // <--- Modified || mode == RplMode.Client 
        {
            RplBotsManagerController controller = NewController(RplIdentity.Local()); // This isn't being run for some reason?
            controller.RplGiven(null);
        }
		
		m_RplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
	  	if (!m_RplComp)
	    	Print("uh oh...");
		if (m_RplComp.Role() == RplRole.Authority)
		{
			if (m_VirtualPlayerManager)
			{
		    	Print("m_VirtualPlayerManager is already here?", LogLevel.WARNING);
			} else {
				m_VirtualPlayerManager        = new CLINTON_VirtualPlayerManager();
			}
			Replication.BumpMe();
		}
        if (mode == RplMode.Client)
		{
			return;
		}/**/ else {
			SetEventMask(owner, EntityEvent.FRAME);
		}
	}
	
	RplBotsManagerController NewController(RplIdentity identity)
    {
        ref Resource controllerPrefab = Resource.Load(s_ControllerPrefab);
		// World world = GetOwner().GetWorld();
		World world = GetGame().GetWorld();
        auto controller = RplBotsManagerController.Cast(
            GetGame().SpawnEntityPrefab(controllerPrefab, world, null));
        controller.m_System = this;
        m_Controllers.Set(identity, controller);

        return controller;
    }

    void DeleteController(RplIdentity identity)
    {
        auto controller = m_Controllers.Get(identity);
        delete controller;
        m_Controllers.Remove(identity);
    }
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		m_fWaitingTime += timeSlice;
		if (m_fWaitingTime < m_iCycleDuration)
			return;

		m_fWaitingTime = 0;
		check_that_the_groups_are_alive();
	}
	
	//------------------------------------------------------------------------------------------------
	void check_that_the_groups_are_alive()
	{
		m_VirtualPlayerManager.check_respawns();
	}

	//------------------------------------------------------------------------------------------------
	void AddBots(string faction, int group_setting, bool customNames)
	{
		Print("authority-side code | AddBots");
		//m_VirtualPlayerManager = CLINTON_VirtualPlayerManager.GetInstance();
		m_VirtualPlayerManager.add_bot( faction, group_setting, customNames);
		Replication.BumpMe();
	}
	
	CLINTON_VirtualPlayerManager GetVirtualPlayerManager()
	{
		return m_VirtualPlayerManager;
	}
}

class RplBotsManagerSessionListener: RplSessionCallbacks
{
    CLINTON_BotsManagerSystem m_System;

    void RplBotsManagerSessionListener(CLINTON_BotsManagerSystem system)
    {
        m_System = system;
    }

    override void EOnConnected(RplIdentity identity)
    {
        RplBotsManagerController controller = m_System.NewController(identity);
        auto rplComponent = BaseRplComponent.Cast(controller.FindComponent(BaseRplComponent));
        rplComponent.Give(identity);
    }

    override void EOnDisconnected(RplIdentity identity)
    {
        m_System.DeleteController(identity);
    }
};

[EntityEditorProps(category: "Tutorial/Entities", description: "This is mine")]
class RplBotsManagerControllerClass : GenericEntityClass {}
RplBotsManagerControllerClass g_RplBotsManagerControllerClassInst;

class RplBotsManagerController : GenericEntity
{
    CLINTON_BotsManagerSystem m_System;
	[RplProp()]
	int m_iDummy; // Test
	
	void RplBotsManagerController(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		Print("Constructer", LogLevel.WARNING);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		Print("EOnInit",LogLevel.WARNING);
		/*
		BaseRplComponent m_RplComp = BaseRplComponent.Cast(this.FindComponent(RplComponent));
		if (!m_RplComp.IsOwner())
			return;
	  	if (!m_RplComp)
	    	Print("uh oh...");
		int read_me = m_RplComp.Id();
		CLINTON_BotMenuUI.botsControllerID = read_me;
		Print(read_me.ToString(),LogLevel.DEBUG);
		*/
	}
	/*
	private bool RplGiven(ScriptBitReader reader)
    {
        Print("PlayerNameInputController.RplGiven()");
        return true;
    } 
	
	void GetRegistered(BaseRplComponent comp)
	{
		if (!comp || !comp.IsOwner())
	    	Print("uh oh...");
		int read_me = comp.Id();
		CLINTON_BotMenuUI.botsControllerID = read_me;
		Print(read_me.ToString(),LogLevel.DEBUG);
	}*/
	
	void RequestAddBots(string faction, int group_setting = 0, bool customNames = false)
	{
		Rpc(Rpc_AddBots_S, faction, group_setting, customNames);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void Rpc_AddBots_S(string faction, int group_setting, bool customNames)
    {
		m_System.AddBots(faction, group_setting, customNames);
	}
	
	bool RplGiven(ScriptBitReader reader)
    {
		BaseRplComponent m_RplComp = BaseRplComponent.Cast(this.FindComponent(RplComponent));
		if (!m_RplComp.IsOwner())
	    	Print("uh oh... ( 2x)");
	  	if (!m_RplComp)
	    	Print("uh oh...");
		int read_me = m_RplComp.Id();
		//CLINTON_BotMenuUI.botsController = read_me;
		Print(read_me.ToString(),LogLevel.DEBUG);
		
        if (false)
        {
            SetEventMask(EntityEvent.FRAME);
        }
        else
        {
            SetEventMask(EntityEvent.FIXEDFRAME);
        }
        return true;
    }
	
	CLINTON_VirtualPlayerManager GetVirtualPlayerManager()
	{
		return m_System.GetVirtualPlayerManager();
	}
};