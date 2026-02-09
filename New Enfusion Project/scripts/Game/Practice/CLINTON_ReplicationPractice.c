[EntityEditorProps(category: "Tutorial/Entities", description: "Lets solve this !:)")]
class CLINTON_ReplicationPracticeClass : GenericEntityClass {}
CLINTON_ReplicationPracticeClass g_RplPracticeClass;

class CLINTON_ReplicationPractice : GenericEntity  // See: https://community.bistudio.com/wiki/Arma_Reforger:Create_an_Entity
{
	private int m_TestValue = 2;  // Lets set this to a random number every two seconds or so
	//protected RplComponent m_RplComp;
	
	void CLINTON_ReplicationPractice(IEntitySource src, IEntity parent)
    {
        this.SetEventMask(EntityEvent.FRAME);
    }
 
    override void EOnFrame(IEntity owner, float timeSlice)
    {
        vector worldTransform[4];
        this.GetWorldTransform(worldTransform);
        Shape.CreateSphere(Color.RED, ShapeFlags.ONCE, worldTransform[3], GetInt());
    }
 
    bool SetInt(int val)
    {
        if (val < 0 || val >= 10)
            return false;
 
        m_TestValue = val;
        return true;
    }
	
	int GetInt() {return m_TestValue;}
	
}

class CLINTON_RplComponentChangeValueClass : ScriptComponentClass {}
CLINTON_RplComponentChangeValueClass g_RplMyComponentClass;

class CLINTON_RplComponentChangeValue : ScriptComponent
{
    // Constant specifying how often (in seconds) to change the color index. For
    // example, setting this to 5 will change the color index every 5 seconds.
    private static const float COLOR_CHANGE_PERIOD_S = 5.0;
	
    // Helper variable for accumulating time (in seconds) every frame and to calculate
    // color index changes.
    private float m_TimeAccumulator_s;
	
	 // Size index currently used for drawing the sphere.
	[RplProp(onRplName: "OnValueChanged")]
	private int m_IntendedVal;
	
	override void OnPostInit(IEntity owner)
    {
		auto shapeEnt = CLINTON_ReplicationPractice.Cast(owner);
        if (!shapeEnt)
        {
            Print("This example requires that the entity is of type `CLINTON_ReplicationPractice`.", LogLevel.WARNING);
            return;
        }

        shapeEnt.SetInt(Math.RandomInt(1,9));
		
		auto rplComponent = BaseRplComponent.Cast(shapeEnt.FindComponent(BaseRplComponent));
        if (!rplComponent)
        {
			Print("ERROROROR ERROR. | CLINTON_RplComponentChangeValue", LogLevel.ERROR);
            //return;
        }
		
        if (rplComponent.Role() == RplRole.Authority)
        {
        	// event handler.
            SetEventMask(owner, EntityEvent.FRAME);
        }
    }
	
	override void EOnFrame(IEntity owner, float timeSlice)
    {
		int nextColour = CalculateColorIdxDelta(timeSlice);
		
		// changeValue()
		if (m_IntendedVal == nextColour) return;
		
		m_IntendedVal = nextColour;
		
		Replication.BumpMe();  // State that the value has changed
		
		CLINTON_ReplicationPractice.Cast(GetOwner()).SetInt(m_IntendedVal);
		//Print("Server code... | CLINTON_RplComponentChangeValue");  // Display on the server if you like
	}
	private int CalculateColorIdxDelta(float timeSlice)
    {
		int nextColour;
		m_TimeAccumulator_s += timeSlice;
		if (m_TimeAccumulator_s > COLOR_CHANGE_PERIOD_S)
		{
			m_TimeAccumulator_s = 0.0;
			return Math.RandomInt(1,9);
		}
		else
		{
			return m_IntendedVal;
		}
	}
	
	void NextColor()
    {
        m_IntendedVal = Math.RandomInt(1,9);
        Replication.BumpMe();
        CLINTON_ReplicationPractice.Cast(GetOwner()).SetInt(m_IntendedVal);
    }
	
	private void OnValueChanged()  // This isn't running
	{
		CLINTON_ReplicationPractice.Cast(GetOwner()).SetInt(m_IntendedVal);
		Print("Client code... | CLINTON_RplComponentChangeValue");  // Display on the server if you like
	}
}

class CLINTON_RplComponentChangeValueTwoClass : ScriptComponentClass {}
CLINTON_RplComponentChangeValueTwoClass g_RplMySecondComponentClass;

class CLINTON_RplComponentChangeValueTwo : ScriptComponent
{
	static const ResourceName s_ControllerPrefab = "{D6F5171E364AF910}Prefabs/RplExampleTwoController_1.et";
    static const ResourceName s_SpherePrefab = "{D0F05369446A7372}Prefabs/RplExampleDebugShape.et";
    // Constant specifying how often (in seconds) to change the color index. For
    // example, setting this to 5 will change the color index every 5 seconds.
    private static const float COLOR_CHANGE_PERIOD_S = 5.0;
	
    // Helper variable for accumulating time (in seconds) every frame and to calculate
    // color index changes.
    private float m_TimeAccumulator_s;
	
    ref RplExampleTwoSessionListener m_SessionListener = new RplExampleTwoSessionListener(this);
    ref map<RplIdentity, RplExampleTwoController> m_Controllers = new map<RplIdentity, RplExampleTwoController>();

    ref array<CLINTON_RplComponentChangeValue> m_Spheres = new array<CLINTON_RplComponentChangeValue>();

	 // Size index currently used for drawing the sphere.
	[RplProp(onRplName: "OnValueChanged")]
	private int m_IntendedVal;
	
	override void OnPostInit(IEntity owner)
    {
        if (g_Game.InPlayMode())
            SetEventMask(owner, EntityEvent.INIT);
		/*
		auto shapeEnt = CLINTON_ReplicationPractice.Cast(owner);
        if (!shapeEnt)
        {
            Print("This example requires that the entity is of type `CLINTON_ReplicationPractice`.", LogLevel.WARNING);
            return;
        }

        shapeEnt.SetInt(Math.RandomInt(1,9));
		
		auto rplComponent = BaseRplComponent.Cast(shapeEnt.FindComponent(BaseRplComponent));
        if (!rplComponent)
        {
			Print("ERROROROR ERROR. | CLINTON_RplComponentChangeValue", LogLevel.ERROR);
            //return;
        }
		
        if (rplComponent.Role() == RplRole.Authority)
        {
        	// event handler.
            SetEventMask(owner, EntityEvent.FRAME);
        }*/
    }
	
    override void EOnInit(IEntity owner)
    {
        RplMode mode = RplSession.Mode();
        if (mode != RplMode.Client)
        {
            RplSession.RegisterCallbacks(m_SessionListener);
        }

        if (mode == RplMode.None || mode == RplMode.Listen)
        {
            RplExampleTwoController controller = NewController(RplIdentity.Local());
            controller.RplGiven(null);
        }

        Resource prefab = Resource.Load(s_SpherePrefab);
        EntitySpawnParams spawnParams = new EntitySpawnParams();
        spawnParams.TransformMode = ETransformMode.WORLD;
        owner.GetWorldTransform(spawnParams.Transform);
        float xBase = spawnParams.Transform[3][0];
        float yBase = spawnParams.Transform[3][1] + 2.0;
        for (int y = -1; y <= 1; y++)
        for (int x = -1; x <= 1; x++)
        {
            spawnParams.Transform[3][0] = xBase + x;
            spawnParams.Transform[3][1] = yBase + y;
            IEntity ent = g_Game.SpawnEntityPrefab(prefab, owner.GetWorld(), spawnParams);
            m_Spheres.Insert(CLINTON_RplComponentChangeValue.Cast(
                ent.FindComponent(CLINTON_RplComponentChangeValue)
            ));
        }
    }
	
    RplExampleTwoController NewController(RplIdentity identity)
    {
        ref Resource controllerPrefab = Resource.Load(s_ControllerPrefab);
        auto controller = RplExampleTwoController.Cast(
            g_Game.SpawnEntityPrefab(controllerPrefab, GetOwner().GetWorld(), null)
        );
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

	/*
	override void EOnFrame(IEntity owner, float timeSlice)
    {
		int nextColour = CalculateColorIdxDelta(timeSlice);
		
		// changeValue()
		if (m_IntendedVal == nextColour) return;
		
		m_IntendedVal = nextColour;
		
		Replication.BumpMe();  // State that the value has changed
		
		CLINTON_ReplicationPractice.Cast(GetOwner()).SetInt(m_IntendedVal);
		//Print("Server code... | CLINTON_RplComponentChangeValue");  // Display on the server if you like
	}
	private int CalculateColorIdxDelta(float timeSlice)
    {
		int nextColour;
		m_TimeAccumulator_s += timeSlice;
		if (m_TimeAccumulator_s > COLOR_CHANGE_PERIOD_S)
		{
			m_TimeAccumulator_s = 0.0;
			return Math.RandomInt(1,9);
		}
		else
		{
			return m_IntendedVal;
		}
	}
	
	private void OnValueChanged()  // This isn't running
	{
		CLINTON_ReplicationPractice.Cast(GetOwner()).SetInt(m_IntendedVal);
		Print("Client code... | CLINTON_RplComponentChangeValue");  // Display on the server if you like
	}*/
	
    void ChangeColor(int idx)
    {
        m_Spheres[idx].NextColor();
    }
	
	private void OnValueChanged()  // This isn't running
	{
		CLINTON_ReplicationPractice.Cast(GetOwner()).SetInt(m_IntendedVal);
		Print("Client code... | CLINTON_RplComponentChangeValue");  // Display on the server if you like
	}
}

class RplExampleTwoSessionListener: RplSessionCallbacks
{
    CLINTON_RplComponentChangeValueTwo m_System;

    void RplExampleTwoSessionListener(CLINTON_RplComponentChangeValueTwo system)
    {
        m_System = system;
    }

    override void EOnConnected(RplIdentity identity)
    {
        RplExampleTwoController controller = m_System.NewController(identity);
        auto rplComponent = BaseRplComponent.Cast(controller.FindComponent(BaseRplComponent));
        rplComponent.Give(identity);
    }

    override void EOnDisconnected(RplIdentity identity)
    {
        m_System.DeleteController(identity);
    }
};

[EntityEditorProps(category: "Tutorial/Entities", description: "Lets solve this !:)")]
class RplExampleTwoControllerClass : GenericEntityClass {}
RplExampleTwoControllerClass g_RplExampleThreeControllerClassInst;

class RplExampleTwoController : GenericEntity
{
    static const KeyCode s_KeyMap[] = {
        KeyCode.KC_NUMPAD1,
        KeyCode.KC_NUMPAD2,
        KeyCode.KC_NUMPAD3,
        KeyCode.KC_NUMPAD4,
        KeyCode.KC_NUMPAD5,
        KeyCode.KC_NUMPAD6,
        KeyCode.KC_NUMPAD7,
        KeyCode.KC_NUMPAD8,
        KeyCode.KC_NUMPAD9,
    };

    CLINTON_RplComponentChangeValueTwo m_System;
    int m_IsDownMask = 0;

    bool RplGiven(ScriptBitReader reader)
    {
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

    override void EOnFrame(IEntity owner, float timeSlice)
    {
        foreach (int idx, KeyCode kc : s_KeyMap)
        {
            int keyBit = 1 << idx;
            bool isDown = Debug.KeyState(kc);
            bool wasDown = (m_IsDownMask & keyBit);
            if (isDown && !wasDown)
                Rpc(Rpc_ChangeColor_S, idx);

            if (isDown)
                m_IsDownMask |= keyBit;
            else
                m_IsDownMask &= ~keyBit;
        }
    }

    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    void Rpc_ChangeColor_S(int idx)
    {
        if (idx < 0 || idx >= 9)
            return;

        m_System.ChangeColor(idx);
    }

    override void EOnFixedFrame(IEntity owner, float timeSlice)
    {
        int isDownMask = 0;
        int keyBit = 1;
        foreach (KeyCode kc : s_KeyMap)
        {
            if (Debug.KeyState(kc))
                isDownMask |= keyBit;

            keyBit <<= 1;
        }
        Rpc(Rpc_OwnerInputs_S, isDownMask);
    }

    [RplRpc(RplChannel.Unreliable, RplRcver.Server)]
    void Rpc_OwnerInputs_S(int isDownMask)
    {
        int inputsChanged = m_IsDownMask ^ isDownMask;
        if (!inputsChanged)
            return;

        for (int idx = 0; idx < 9; idx++)
        {
            int keyBit = 1 << idx;
            bool isDown = isDownMask & keyBit;
            bool wasDown = m_IsDownMask & keyBit;
            if (isDown && !wasDown)
                m_System.ChangeColor(idx);
        }

        m_IsDownMask = isDownMask;
    }
};