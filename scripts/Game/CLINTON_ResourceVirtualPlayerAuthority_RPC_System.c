// Works for every frame (upto 20:45 of https://www.youtube.com/watch?v=rTqb3QvndxA), but maybe not needed because we are not periodically needing updates

class CLINTON_ResourceVirtualPlayerAuthority_RPC_System : GameSystem
{
	protected ref array<SCR_ResourceComponent> m_aResourceComponents;
	protected ref array<ref CLINTON_Virtual_Player> m_aResourceVirtualPlayers;
	
	array<ref CLINTON_Virtual_Player> GetResourceVirtualPlayers()
	{
		return m_aResourceVirtualPlayers;
	}
	
	[RplRpc(channel : RplChannel.Unreliable, rcver : RplRcver.Broadcast)]
	void Rpc_SetResourceVirtualPlayers(array<ref CLINTON_Virtual_Player> virtualPlayers)
	{
		m_aResourceVirtualPlayers = virtualPlayers;
	}
	
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo.SetAbstract(true).SetUnique(true);
	}
	
	override protected void OnUpdate(ESystemPoint point)
	{
		super.OnUpdate(point);
		
		const RplId systemRplId 	= Replication.FindItemId(this);
		const RplNode systemRplNode = Replication.FindNode(systemRplId);
		
		if (systemRplNode.GetRole() == RplRole.Proxy)
			return;
		
		if (point == ESystemPoint.FixedFrame)
			UpdateRadarBlips();  // Assuming change this
	}
	/*
	void UpdateRadarBlips()
	{
		return;
	}*/
	
	override event protected void OnStarted()
	{
		m_aResourceComponents = new array<SCR_ResourceComponent>();
	}
	
	void RegisterResourceComponent(notnull SCR_ResourceComponent component)
	{
		if (m_aResourceComponents.Contains(component))
			return;
		
		m_aResourceComponents.Insert(component);
	}
	
	void UnregisterResourceComponent(notnull SCR_ResourceComponent component)
	{
		m_aResourceComponents.RemoveItem(component);
	}
	
	protected void UpdateRadarBlips()
	{
		if (!m_aResourceVirtualPlayers)
		m_aResourceVirtualPlayers = new array<ref CLINTON_Virtual_Player>();
		
		const int oldSize = m_aResourceVirtualPlayers.Count();
		const int newSize = m_aResourceComponents.Count();
		
		m_aResourceVirtualPlayers.Resize(newSize);
		
		for (int i = 0; i < newSize; ++i)
		{
			CLINTON_Virtual_Player playerData = m_aResourceVirtualPlayers[i];
			
			if (!playerData)
			{
				playerData = new CLINTON_Virtual_Player();
				m_aResourceVirtualPlayers[i] = playerData;
			}
			
			SCR_ResourceComponent component = m_aResourceComponents[i];
			
			//playerData.SetPosition(component.GetOwner().GetOrigin());
			//playerData.SetResourceComponent(component);
		}
		
		Rpc(Rpc_SetResourceVirtualPlayers, m_aResourceVirtualPlayers);
	}
}