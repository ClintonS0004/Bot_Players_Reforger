class SCR_MapsBotsMenuUI : SCR_MapUIBaseComponent
{
	[Attribute("ToolMenu", UIWidgets.EditBox, desc: "Root frame widget name")]
	string m_sToolMenuRootName;
	
	[Attribute("exclamationCircle", desc: "Toolmenu imageset quad name")]
	protected string m_sToolMenuIconName;
	
	[Attribute("JournalFrame", desc: "Root frame widget name")]
	protected string m_sRootWidgetName;
	
	[Attribute("{01C52F2B014A2BAC}MyLayout.layout", desc: "Journal layout path")]
	protected ResourceName m_sJournalLayout;

	protected Widget m_wJournalFrame;
	protected SCR_MapToolEntry m_ToolMenuEntry;
	protected SCR_MapToolMenuUI m_ToolMenu;
	
	override void Init()
	{
		
		m_wJournalFrame = m_RootWidget.FindAnyWidget(m_sRootWidgetName);
		if (!m_wJournalFrame)
			return;
		
		m_ToolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (m_ToolMenu)
		{
			m_ToolMenuEntry = m_ToolMenu.RegisterToolMenuEntry(SCR_MapToolMenuUI.s_sToolMenuIcons, m_sToolMenuIconName, 10);
			m_ToolMenuEntry.m_OnClick.Insert(send_message);
			//m_ToolMenuEntry.SetEnabled(true);
		}
	}
	
	void send_message()
	{
		if (!m_ToolMenuEntry || !m_ToolMenu)
			return;
		Print("Hello");
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.SCR_MapsBotsMenuUI); 
		/*
		bool visible = m_wJournalFrame.IsVisible();
		m_wJournalFrame.SetVisible(!visible);
		if (m_ToolMenuEntry)
			m_ToolMenuEntry.SetActive(!visible);
		
		Print("Hello");
		// GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.SCR_MapsBotsMenuUI); 
		//GetGame().GetMenuManager().
		//SCR_MapEntity.GetMapInstance().GetMapMenuRoot();
		//SCR_MapEntity.GetMapInstance().GetMapUIComponent();
		//SCR_MapEntity.GetMapInstance().GetMapWidget();
		
		//SCR_MapEntity.GetMapInstance().AddChild();
		Widget mapMenu = m_RootWidget.FindAnyWidget("BotFrame");
		
		mapMenu.AddChild(m_wJournalFrame);
		mapMenu.SetEnabled(true);
		
		visible = mapMenu.IsVisible();
		mapMenu.SetVisible(!visible);
		if (m_ToolMenuEntry)
			m_ToolMenuEntry.SetActive(!visible);
		mapMenu.SetVisible(true);
		//  GUI       (E): Cannot add a child, the MapWidget MapWidget does not accept more children
 		*/return;
	}
	/*
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		// m_wJournalFrame = m_RootWidget.FindAnyWidget(m_sRootWidgetName);
		Widget m_wJournalFrame = m_RootWidget.FindAnyWidget("BotFrame");
		if (!m_wJournalFrame)
			m_wJournalFrame = m_RootWidget.FindAnyWidget(m_sRootWidgetName);
			// return;
		
		if (!m_wJournalFrame.GetChildren())
		{
			Widget journal = GetGame().GetWorkspace().CreateWidgets(m_sJournalLayout, m_wJournalFrame);
			
			if (!journal)
				return;
		}


		//m_PlyFactionAffilComp.GetOnPlayerFactionResponseInvoker_O().Insert(OnPlayerFactionResponse);

		//GetJournalForPlayer();		
		
		send_message();
		
	}*/
}
