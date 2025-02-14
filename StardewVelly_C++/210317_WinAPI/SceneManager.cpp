#include "SceneManager.h"
#include "GameNode.h"

GameNode* SceneManager::lastScene = nullptr;
GameNode* SceneManager::currentScene = nullptr;
GameNode* SceneManager::loadingScene = nullptr;
GameNode* SceneManager::readyScene = nullptr;

DWORD CALLBACK LoadingThread(LPVOID pvParam)
{
    if (SUCCEEDED(SceneManager::readyScene->Init()))
    {
        SceneManager::currentScene = SceneManager::readyScene;

        SceneManager::loadingScene->Release();
        SceneManager::loadingScene = nullptr;
        SceneManager::readyScene = nullptr;
    }
    return 0;
}

HRESULT SceneManager::Init()
{
    return S_OK;
}

void SceneManager::Release()
{
    map<string, GameNode*>::iterator it;
    for (it = mSceneDatas.begin(); it != mSceneDatas.end(); it++)
    {
        if (it->second)
        {
            SAFE_RELEASE(it->second);
        }
    }
    mSceneDatas.clear();

    ReleaseSingleton();
}

void SceneManager::Update()
{
    if (currentScene)
    {
        currentScene->Update();
    }
}

void SceneManager::Render(HDC hdc)
{
    if (currentScene)
    {
        currentScene->Render(hdc);
    }
}

GameNode* SceneManager::AddScene(string key, GameNode* scene)
{
    if (scene == nullptr)
    {
        return nullptr;
    }

    map<string, GameNode*>::iterator it = mSceneDatas.find(key);
    if (it != mSceneDatas.end())
    {
        return it->second;
    }

    scene->SetID(mSceneDatas.size());
    mSceneDatas.insert(pair<string, GameNode*>(key, scene));

    return scene;
}

GameNode* SceneManager::AddLoadingScene(string key, GameNode* scene)
{
    if (scene == nullptr)
    {
        return nullptr;
    }

    map<string, GameNode*>::iterator it = mLoadingSceneDatas.find(key);
    if (it != mLoadingSceneDatas.end())
    {
        return it->second;
    }

    mLoadingSceneDatas.insert(pair<string, GameNode*>(key, scene));

    return scene;
}

HRESULT SceneManager::ChangeScene(string sceneName)
{
    lastScene = currentScene;

    map<string, GameNode*>::iterator it = mSceneDatas.find(sceneName);
    if (it == mSceneDatas.end())
    {
        return E_FAIL;
    }

    if (it->second == currentScene)
    {
        return S_OK;
    }

    if (SUCCEEDED(it->second->Init()))
    {
        // 현재 씬 -> 타이틀 씬
        // 바꾸고 싶은 씬 -> 배틀 씬
        if (currentScene)
        {
            currentScene->Release();
        }
        currentScene = it->second;

        return S_OK;
    }

    return E_FAIL;
}

HRESULT SceneManager::ChangeScene(string sceneName, string LoadingsceneName)
{
    map<string, GameNode*>::iterator it = mSceneDatas.find(sceneName);
    if (it == mSceneDatas.end())
    {
        return E_FAIL;
    }

    if (it->second == currentScene)
    {
        return S_OK;
    }

    //로딩씬 찾기
    map<string, GameNode*>::iterator itLoading = mLoadingSceneDatas.find(LoadingsceneName);

    if (itLoading == mLoadingSceneDatas.end())
    {
        return ChangeScene(sceneName);
    }

    if (SUCCEEDED(itLoading->second->Init()))
    {
        if (currentScene)
        {
            GameData::GetSingleton()->SetLastSceneID(currentScene->GetID());
            currentScene->Release();
        }
        currentScene = itLoading->second;

        readyScene = it->second;
        loadingScene = itLoading->second;

        // 다음 씬을 초기화할 쓰레드를 생성
        DWORD loadingThreadId;
        HANDLE hThread;
        hThread = CreateThread(NULL, 0,
            LoadingThread,  //실행 시킬 함수포인터
            NULL,           // 실행 시킬 함수에 들어갈 매개변수
            0,
            &loadingThreadId
        );

        CloseHandle(hThread);

        return S_OK;
    }

    return E_NOTIMPL;
}

