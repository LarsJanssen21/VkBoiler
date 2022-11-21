
#include "Client/MyApp.h"

VulkanApp* CreateApp() { return new MyApp; }

class AppHandler
{
public:
	explicit AppHandler(VulkanApp* pApp)
		:m_pApp(pApp)
	{

	}

	void Init()
	{
		m_pApp->BaseInit();
		if(!m_pApp->m_InitializedBase)
			m_pApp->Init();
	}
	void Tick()
	{
		m_pApp->Tick();
		m_pApp->WindowUpdate();
		m_pApp->m_Running = !m_pApp->m_pWindow->WantsQuit();
	}
	void Destroy() 
	{
		if (!m_pApp->m_InitializedBase)
			m_pApp->Destroy();

		m_pApp->BaseDestroy();
	}

	bool KeepRunning() const { return m_pApp->m_Running; }

private:
	VulkanApp* m_pApp;
};


/*--------------*/
/*	Entrypoint	*/
/*--------------*/
int main()
{
	AppHandler handler(CreateApp());

	handler.Init();

	while (handler.KeepRunning())
	{
		handler.Tick();
	}

	handler.Destroy();

	return 0;
}