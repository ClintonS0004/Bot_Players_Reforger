[EntityEditorProps(category: "Tutorial/Entities", description: "Lets solve this !:)")]
class RplExampleDebugShapeClass: GenericEntityClass {}
RplExampleDebugShapeClass g_RplExampleDebugShapeClassInst;

class RplExampleDebugShape : GenericEntity  // See: file:///C:/Program%20Files%20(x86)/Steam/steamapps/common/Arma%20Reforger%20Tools/Workbench/docs/EnfusionScriptAPIPublic/Page_Replication_Overview.html#Codecs
{
    static const int COLOR_COUNT = 4;
    static const int COLORS[] = {
        Color.BLACK,
        Color.RED,
        Color.GREEN,
        Color.BLUE,
    };

    private int m_Color;

    void RplExampleDebugShape(IEntitySource src, IEntity parent)
    {
        this.SetEventMask(EntityEvent.FRAME);
    }

    override void EOnFrame(IEntity owner, float timeSlice)
    {
        vector worldTransform[4];
        this.GetWorldTransform(worldTransform);
        Shape.CreateSphere(m_Color, ShapeFlags.ONCE, worldTransform[3], 0.5);
    }

    bool SetColorByIdx(int colorIdx)
    {
        if (colorIdx < 0 || colorIdx >= COLOR_COUNT)
            return false;

        m_Color = COLORS[colorIdx];
        return true;
    }
}

class RplExample1ComponentColorAnimClass : ScriptComponentClass { }
RplExample1ComponentColorAnimClass g_RplExample1ComponentColorAnimClass;

class RplExample1ComponentColorAnim : ScriptComponent
{
    // Constant specifying how often (in seconds) to change the color index. For
    // example, setting this to 5 will change the color index every 5 seconds.
    private static const float COLOR_CHANGE_PERIOD_S = 5.0;

    // Helper variable for accumulating time (in seconds) every frame and to calculate
    // color index changes.
    private float m_TimeAccumulator_s;

    // Color index currently used for drawing the sphere.
    private int m_ColorIdx;

    override void OnPostInit(IEntity owner)
    {
        // We check whether this component is attached to entity of correct type and
        // report a problem if not. Once this test passes during initialization, we
        // do not need to worry about owner entity being wrong type anymore.
        auto shapeEnt = RplExampleDebugShape.Cast(owner);
        if (!shapeEnt)
        {
            Print("This example requires that the entity is of type `RplExampleDebugShape`.", LogLevel.WARNING);
            return;
        }

        // We initialize shape entity to correct color.
        shapeEnt.SetColorByIdx(m_ColorIdx);

        // We subscribe to "frame" events, so that we can run our logic in `EOnFrame`
        // event handler.
        SetEventMask(owner, EntityEvent.FRAME);
    }

    override void EOnFrame(IEntity owner, float timeSlice)
    {
        // We calculate change of color index based on time (and configured color
        // change period), then apply the change in color.
        int colorIdxDelta = CalculateColorIdxDelta(timeSlice);
        ApplyColorIdxDelta(owner, colorIdxDelta);
    }

    private int CalculateColorIdxDelta(float timeSlice)
    {
        // We first accumulate time and then calculate how many color change periods
        // have occurred, giving us number of colors we've cycled through.
        m_TimeAccumulator_s += timeSlice;
        int colorIdxDelta = m_TimeAccumulator_s / COLOR_CHANGE_PERIOD_S;

        // We remove full periods from the accumulator, only carrying over how much
        // time from current period has elapsed.
        m_TimeAccumulator_s -= colorIdxDelta * COLOR_CHANGE_PERIOD_S;

        return colorIdxDelta;
    }

    private void ApplyColorIdxDelta(IEntity owner, int colorIdxDelta)
    {
        // If there is no change to color index, we do nothing.
        if (colorIdxDelta == 0)
            return;

        // We calculate new color index.
        int newColorIdx = (m_ColorIdx + colorIdxDelta) % RplExampleDebugShape.COLOR_COUNT;

        // We check also new color index, since shorter periods and lower frame-rate
        // may result in new and old color index values being the same.
        if (newColorIdx == m_ColorIdx)
            return;

        // Now we can update the color index ...
        m_ColorIdx = newColorIdx;

        // ... and set new color based on new color index value.
        RplExampleDebugShape.Cast(owner).SetColorByIdx(m_ColorIdx);
    }
}

class RplExample2ComponentColorAnimClass : ScriptComponentClass { }
RplExample2ComponentColorAnimClass g_RplExample2ComponentColorAnimClass;

class RplExample2ComponentColorAnim : ScriptComponent
{
    private static const float COLOR_CHANGE_PERIOD_S = 5.0;

    private float m_TimeAccumulator_s;

    // We mark color index as replicated property using RplProp attribute, making
    // it part of replicated state. We also say we want OnColorIdxChanged function
    // to be invoked whenever replication updates value of color index.
    [RplProp(onRplName: "OnColorIdxChanged")]
    private int m_ColorIdx;

    override void OnPostInit(IEntity owner)
    {
        auto shapeEnt = RplExampleDebugShape.Cast(owner);
        if (!shapeEnt)
        {
            Print("This example requires that the entity is of type `RplExampleDebugShape`.", LogLevel.WARNING);
            return;
        }

        shapeEnt.SetColorByIdx(m_ColorIdx);

        // We must belong to some RplComponent in order for replication to work.
        // We search for it and warn user when we can't find it.
        auto rplComponent = BaseRplComponent.Cast(shapeEnt.FindComponent(BaseRplComponent));
        if (!rplComponent)
        {
            Print("This example requires that the entity has an RplComponent.", LogLevel.WARNING);
            return;
        }

        // We only perform simulation on the authority instance, while all proxy
        // instances just show result of the simulation. Therefore, we only have to
        // subscribe to "frame" events on authority, leaving proxy instances as
        // passive components that do something only when necessary.
        if (rplComponent.Role() == RplRole.Authority)
        {
            SetEventMask(owner, EntityEvent.FRAME);
        }
    }

    override void EOnFrame(IEntity owner, float timeSlice)
    {
        int colorIdxDelta = CalculateColorIdxDelta(timeSlice);
        ApplyColorIdxDelta(owner, colorIdxDelta);
    }

    private int CalculateColorIdxDelta(float timeSlice)
    {
        m_TimeAccumulator_s += timeSlice;
        int colorIdxDelta = m_TimeAccumulator_s / COLOR_CHANGE_PERIOD_S;
        m_TimeAccumulator_s -= colorIdxDelta * COLOR_CHANGE_PERIOD_S;
        return colorIdxDelta;
    }

    private void ApplyColorIdxDelta(IEntity owner, int colorIdxDelta)
    {
        if (colorIdxDelta == 0)
            return;

        int newColorIdx = (m_ColorIdx + colorIdxDelta) % RplExampleDebugShape.COLOR_COUNT;
        if (newColorIdx == m_ColorIdx)
            return;

        // Update replicated state with results from the simulation.
        m_ColorIdx = newColorIdx;

        // After we have written new value of color index, we let replication know
        // that there are changes in our state that need to be replicated to proxies.
        // Without this call, even if we change our color index, new value would not
        // be replicated to proxies.
        Replication.BumpMe();

        // Presentation of replicated state on authority.
        RplExampleDebugShape.Cast(owner).SetColorByIdx(m_ColorIdx);
    }

    // Presentation of replicated state on proxy.
    private void OnColorIdxChanged()
    {
        RplExampleDebugShape.Cast(GetOwner()).SetColorByIdx(m_ColorIdx);
    }
}

class RplExample3ComponentColorAnimClass : ScriptComponentClass { }
RplExample3ComponentColorAnimClass g_RplExample3ComponentColorAnimClass;

class RplExample3ComponentColorAnim : ScriptComponent
{
    [RplProp(onRplName: "OnColorIdxChanged")]
    private int m_ColorIdx;

    override void OnPostInit(IEntity owner)
    {
        auto shapeEnt = RplExampleDebugShape.Cast(owner);
        if (!shapeEnt)
        {
            Print("This example requires that the entity is of type `RplExampleDebugShape`.", LogLevel.WARNING);
            return;
        }

        shapeEnt.SetColorByIdx(m_ColorIdx);

        auto rplComponent = BaseRplComponent.Cast(shapeEnt.FindComponent(BaseRplComponent));
        if (!rplComponent)
        {
            Print("This example requires that the entity has an RplComponent.", LogLevel.WARNING);
            return;
        }
    }

    void NextColor()
    {
        m_ColorIdx = (m_ColorIdx + 1) % RplExampleDebugShape.COLOR_COUNT;
        Replication.BumpMe();
        RplExampleDebugShape.Cast(GetOwner()).SetColorByIdx(m_ColorIdx);
    }

    private void OnColorIdxChanged()
    {
        RplExampleDebugShape.Cast(GetOwner()).SetColorByIdx(m_ColorIdx);
    }
}

class RplExample3SystemClass : ScriptComponentClass { }
RplExample3SystemClass g_RplExample3SystemClassInst;

class RplExample3System : ScriptComponent
{
    static const ResourceName s_ControllerPrefab = "{AE2BF54C327A0025}Prefabs/RplExampleController.et";
    static const ResourceName s_SpherePrefab = "{18C641DA6FB5F97A}Prefabs/RplExampleShape.et";

    ref RplExample3SessionListener m_SessionListener = new RplExample3SessionListener(this);
    ref map<RplIdentity, RplExample3Controller> m_Controllers = new map<RplIdentity, RplExample3Controller>();

    ref array<RplExample3ComponentColorAnim> m_Spheres = new array<RplExample3ComponentColorAnim>();

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

        if (mode == RplMode.None || mode == RplMode.Listen)
        {
            RplExample3Controller controller = NewController(RplIdentity.Local());
            controller.RplGiven(null);
        }
		
        if (mode == RplMode.Client)
		{
			return;
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
            IEntity ent = GetGame().SpawnEntityPrefab(prefab, GetGame().GetWorld(), spawnParams);  // g_Game
            m_Spheres.Insert(RplExample3ComponentColorAnim.Cast(
                ent.FindComponent(RplExample3ComponentColorAnim)
            ));
        }
    }

    RplExample3Controller NewController(RplIdentity identity)
    {
        ref Resource controllerPrefab = Resource.Load(s_ControllerPrefab);
        auto controller = RplExample3Controller.Cast(
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

    void ChangeColor(int idx)
    {
        m_Spheres[idx].NextColor();
    }
}

class RplExample3SessionListener: RplSessionCallbacks
{
    RplExample3System m_System;

    void RplExample3SessionListener(RplExample3System system)
    {
        m_System = system;
    }

    override void EOnConnected(RplIdentity identity)
    {
        RplExample3Controller controller = m_System.NewController(identity);
        auto rplComponent = BaseRplComponent.Cast(controller.FindComponent(BaseRplComponent));
        rplComponent.Give(identity);
    }

    override void EOnDisconnected(RplIdentity identity)
    {
        m_System.DeleteController(identity);
    }
};

[EntityEditorProps(category: "Tutorial/Entities", description: "This is the example")]
class RplExample3ControllerClass : GenericEntityClass {}
RplExample3ControllerClass g_RplExample3ControllerClassInst;

class RplExample3Controller : GenericEntity
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

    RplExample3System m_System;
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