class BotsRespawningSystem : BaseSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
    {
        // 1.
        outInfo
            .SetAbstract(false)
            .SetLocation(ESystemLocation.Server)
            .AddPoint(ESystemPoint.Frame);
    }
	    
	private int m_FrameCount = 0;
 
    override void OnUpdate(ESystemPoint point)
    {
        if (point == ESystemPoint.Frame)
        {
            int frameIndex = m_FrameCount;
            //PrintFormat("Hello world (systems) frame %1", frameIndex);
 
            m_FrameCount += 1;
 
			// Every 512 frames
            if (m_FrameCount == 512)
			{
                //this.Enable(false);
				m_FrameCount = 0;
				
				WorldSystem m_system = WorldSystem.Cast(this);
				if (!m_system) 
					Print("NNNNOOOOOOOO!",LogLevel.ERROR);
				BotsWorldController controller = BotsWorldController.Cast(m_system.GetSystems().FindMyController(BotsWorldController));
				BotsWorldSystem systemTwo = BotsWorldSystem.Cast( GetGame().GetWorld().FindSystem(BotsWorldSystem));
				CLINTON_VirtualPlayerManager pm = systemTwo.m_VirtualPlayerManager;
				if (!pm)
					return;
				pm.check_respawns();
			}
		}
    }
}