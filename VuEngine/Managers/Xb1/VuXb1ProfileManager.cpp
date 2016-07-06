//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xb1 ProfileManager class
// 
//*****************************************************************************
#include "VuEngine/Managers/Xb1/VuXb1ProfileManager.h"
#include "VuEngine/HAL/GamePad/Xb1/VuXb1GamePad.h"

using namespace Windows::Xbox;
using namespace Windows::Xbox::UI;
using namespace Windows::Xbox::System;

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuProfileManager, VuXb1ProfileManager);

//*****************************************************************************
bool VuXb1ProfileManager::init(const std::string &gameName)
{
	// Must have an active user, disallow playing the game (fail init() with false)
	// if they don't have a valid user signed in
	//
	User^ signedInUser = nullptr;
	extern std::string gRestartUserId;

	// Let the RestartUser override the default (controller-paired) user if we're
	// in a restart situation.
	if (gRestartUserId != "")
	{
		// Find the user in the list
		for (int i=0; i < (int)User::Users->Size; i++)
		{
			User^ u = User::Users->GetAt(i);

			std::wstring uStr = u->XboxUserId->Data();
			std::wstring wideUserId(gRestartUserId.begin(), gRestartUserId.end());

			if (isSameUserId(uStr.c_str(), wideUserId.c_str()))
			{
				// Found the same user
				signedInUser = u;
				break;
			}
		}
	}
	else
	{
		// Check the controllers to see if the Primary user
		// has already been identified, and use them.
		//
		auto primary = VuXb1GamePad::IF()->getPrimaryGamepad();
		if (primary != nullptr && primary->User != nullptr)
		{
			signedInUser = primary->User;
		}
	}

	// If anyone is signed in, then use them
	//
	if (signedInUser == nullptr)
	{
		auto users = User::Users;
		for (auto user : users)
		{
			if (user->IsSignedIn)
			{
				signedInUser = user;
				break;
			}
		}
	}

	// If we still haven't found a signed-in user, then identify one now.
	// Will not allow the player to continue until they've selected a 
	// User via the picker
	//
	if (signedInUser == nullptr)
	{
		UI::AccountPickerOptions options = UI::AccountPickerOptions::None;

		// Whole thing in try block in case that user hits the Xbox Home Button
		// and .wait() throws an exception.
		//
		try
		{
			while (signedInUser == nullptr)
			{
				auto picker = SystemUI::ShowAccountPickerAsync(nullptr, options);
				auto pickerTask = Concurrency::create_task(picker);
	
				pickerTask.then([&signedInUser](UI::AccountPickerResult^ result)
				{
					try
					{
						if (result->User)
						{
							auto users = User::Users;

							for (auto user : users)
							{
								if (user == result->User)
								{
									signedInUser = user;
									break;
								}
							}
						}
					}

					catch (Platform::Exception^ ex)
					{
						VUWPRINTF(L"Error: Could not get a valid user.\n");
					}
				}).wait();
			}
		}

		catch (Platform::Exception^ ex)
		{
			VUWPRINTF(L"Error: User dismissed picker with Home button.\n");

			return false;
		}
	}

	// Initialize the storage container for this User
	//
	setUserAndLoad(signedInUser);
	mUser = signedInUser;

	// Continue init as per normal now that we can access the storage
	// container for profile data
	return VuProfileManager::init(gameName);
}

//*****************************************************************************
void  VuXb1ProfileManager::setUserAndLoad(User^ user) 
{ 
	if (user != mUser)
	{
		IAsyncOperation<ConnectedStorageSpace^>^ op = ConnectedStorageSpace::GetForUserAsync(user);

		create_task(op).then(
			[=] (task<ConnectedStorageSpace^> operation)
		{
			try
			{
				mConnectedStorageSpace = operation.get();
				mUser = user;
				loadInternal();
			}

			catch (Platform::COMException^ ex)
			{
				// Fail
				VUWPRINTF(L"Exception: (%#.8x) Unable to initialize ConnectedStorage for user %s. Likely a bad SCID in the AppXManifest.\n", ex->HResult, user->XboxUserId->Data());
			}
		}
		).wait();
	}
}

//*****************************************************************************
void VuXb1ProfileManager::loadInternal()
{
	if (mUser == nullptr)
	{
		return;
	}

	int bufferSize = 128*1024;

	Streams::Buffer^ buffer = ref new Streams::Buffer(bufferSize);

	auto reads = ref new Platform::Collections::Map<Platform::String^, Streams::IBuffer^>();
	reads->Insert("data", buffer);

	auto container = mConnectedStorageSpace->CreateContainer("Profile");

	auto op = container->ReadAsync(reads->GetView());
	op->Completed = ref new AsyncActionCompletedHandler(
		[=] (IAsyncAction^ a, Windows::Foundation::AsyncStatus status)
	{
		critical_section::scoped_lock lock(mLock);

		// Call Completion function
		switch (status)
		{
		case Windows::Foundation::AsyncStatus::Completed:
			// Success
			{
				// Calculate sizes
				int profileSize = buffer->Length;
				int headerSize = sizeof(ProfileHeader);
				int dataSize = profileSize - headerSize;

				// Allocate memory and set up pointers
				char *pRawData = new char[profileSize];
				ProfileHeader *pHeaderData = (ProfileHeader*)pRawData;
				char *pData = pRawData + headerSize;

				// Copy data
				byte *pBufferPtr = getBufferPointer(buffer);
				VU_MEMCPY(pRawData, profileSize, pBufferPtr, profileSize);

				// verify hash
				VUUINT32 hash = VuHash::fnv32(pData, dataSize);
				if (pHeaderData->mDataHash == hash)
				{
					// read json data from memory
					VuJsonBinaryReader reader;
		
					if ( reader.loadFromMemory(mData, pData, dataSize) )
					{
						// Success
						VUWPRINTF(L"Status: Xb1 Profile read for %s.\n", mUser->XboxUserId->Data());
					}
				}

				// clean up
				delete[] pRawData;
			}
			break;

		case Windows::Foundation::AsyncStatus::Error:
			VUWPRINTF(L"ERROR: Connected Storage Space Container ReadASync() returned an error (%#.8x '%s')\n", a->ErrorCode, a->ToString()->Data());
			break;
		case Windows::Foundation::AsyncStatus::Canceled:
			// Failure
			break;
		}
	});
}

//*****************************************************************************
void VuXb1ProfileManager::saveInternal()
{
	if (mUser == nullptr)
	{
		return;
	}

	// calculate sizes
	int headerSize = sizeof(ProfileHeader);
	int dataSize = VuJsonBinaryWriter::calculateDataSize(mData);
	int bufferSize = headerSize + dataSize;

	// allocate memory for header and data
	char *pRawData = new char[bufferSize];

	// Header points to the start of memory
	ProfileHeader* pHeader = (ProfileHeader*)pRawData;
	char *pProfileData = pRawData + headerSize;

	// write data to memory
	VuJsonBinaryWriter writer;
	if (writer.saveToMemory(mData, pProfileData, dataSize))
	{
		// fill in header
		pHeader->mMagic = scProfileMagic;
		pHeader->mVersion = PROFILE_VERSION;
		pHeader->mDataSize = dataSize;
		pHeader->mDataHash = VuHash::fnv32(pProfileData, dataSize);

		// Create a Buffer object to hold header + data
		Streams::Buffer^ buffer = ref new Streams::Buffer(bufferSize);
		buffer->Length = bufferSize;

		// Copy the data into the Buffer object
		byte* pBufferPtr = getBufferPointer(buffer);
		VU_MEMCPY(pBufferPtr, bufferSize, pRawData, bufferSize);

		// Get a Container called "Profile"
		auto container = mConnectedStorageSpace->CreateContainer("Profile");

		// Create a Map to stick the buffer into
		auto updates = ref new Platform::Collections::Map<Platform::String^, Streams::IBuffer^>();

		// This blob is called "data"
		updates->Insert("data", buffer);

		auto op = container->SubmitUpdatesAsync(updates->GetView(), nullptr);

		op->Completed = ref new AsyncActionCompletedHandler(
			[=] (IAsyncAction^ a, Windows::Foundation::AsyncStatus status)
		{
			// Call Completion function
			switch (status)
			{
			case Windows::Foundation::AsyncStatus::Completed:
				// Success
				VUWPRINTF(L"Status: Xb1 Profile written for %s.\n", mUser->XboxUserId->Data());
				break;

			case Windows::Foundation::AsyncStatus::Error:
			case Windows::Foundation::AsyncStatus::Canceled:
				// Failure
				break;
			}
		});
	}

	delete[] pRawData;
}

//*****************************************************************************
// Holy fuckdown, batman. To get a raw pointer from an IBuffer, we have to
// downcast to IUnknown.
//
byte* VuXb1ProfileManager::getBufferPointer(Streams::IBuffer^ buffer)
{
    IUnknown* unknown = reinterpret_cast<IUnknown*>(buffer);

	Microsoft::WRL::ComPtr<Streams::IBufferByteAccess> bufferByteAccess;
    
	HRESULT hr = unknown->QueryInterface(_uuidof(Streams::IBufferByteAccess), &bufferByteAccess);
	if (FAILED(hr))
	{
		return nullptr;
	}

	byte* bytes = nullptr;
    bufferByteAccess->Buffer(&bytes);

    return bytes;
}

//*****************************************************************************
bool VuXb1ProfileManager::isSameUserId(const wchar_t* user1, const wchar_t* user2)
{
	return (_wcsicmp(user1, user2) == 0);
}

//*****************************************************************************
bool VuXb1ProfileManager::isSameUserId(const wchar_t* user1, IUser^ user2)
{
	if (user2 == nullptr)
	{
		return false;
	}

	return isSameUserId(user1, user2->XboxUserId->Data());
}

//*****************************************************************************
bool VuXb1ProfileManager::isSameUserId(IUser^ user1, const wchar_t* user2)
{
	if (user1 == nullptr)
	{
		return false;
	}

	return isSameUserId(user1->XboxUserId->Data(), user2);
}

//*****************************************************************************
bool VuXb1ProfileManager::isSameUserId(IUser^ user1, IUser^ user2)
{
	if (user1 == nullptr || user2 == nullptr)
	{
		return false;
	}

	return isSameUserId(user1->XboxUserId->Data(), user2->XboxUserId->Data());
}
