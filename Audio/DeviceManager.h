
#pragma once

#include "Audio.h"
#include "Device.h"
#include <vector>
#include <functional>

namespace Audio
{
	class DeviceManager
	{
	public:
		using OnChange = std::function<void()>;
		using OnFatalError = std::function<void(std::wstring message)>;

		/**
		 * @brief 
		 * @param onChange Callback upon something about the device list changing. This is lazy, but given the scope of this app, works.
		 * @param onFatalError Callback upon an unrecoverable error occurring. Again, using this due to the small scope of this application - COM failures aren't really that "recoverable" I would imagine, since this app has only one real function anyway.
		 */
		DeviceManager(const OnChange onChange, const OnFatalError onFatalError);

		/**
		 * @brief Refreshes the list of audio devices.
		 * @return An error, if one occurred during the operation.
		 */
		[[nodiscard]]
		const std::optional<Error> refresh();

		/**
		 * @brief Gets an audio device by its unique ID.
		 * @param id Opaque identifier of the audio device.
		 * @return The audio device associated with the provided ID.
		 */
		const Device& operator[](Device::Id id) const;

		/**
		 * @brief Gets an audio device by its unique ID.
		 * @param id Opaque identifier of the audio device.
		 * @return The audio device associated with the provided ID.
		 */
		Device& operator[](Device::Id id);

		/**
		 * @brief Gets the audio device that's been set as the default.
		 * @return The default audio device.
		 */
		const Device& getDefault(ERole role) const;

		/**
		 * @brief Sets the default audio device for all roles.
		 * @param device The device to set as the default.
		 */
		std::optional<Error> setDefault(const Device& device);

		/**
		 * @brief Sets the default audio device for all roles.
		 * @param id The ID of the device to be the default.
		 */
		std::optional<Error> setDefault(Device::Id id);

		std::vector<Device> devices;

	private:

		/**
		 * @brief Client to notify us of any changes to audio devices,
		 * we wrap this into a nicer interface for the consumer that uses our `Device` type.
		 */
		class NotificationClient : public IMMNotificationClient
		{
		public:
			NotificationClient(DeviceManager* deviceManager)
				: deviceManager(deviceManager) {}

			HRESULT __stdcall OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
			HRESULT __stdcall OnDeviceAdded(LPCWSTR pwstrDeviceId);
			HRESULT __stdcall OnDeviceRemoved(LPCWSTR pwstrDeviceId);
			HRESULT __stdcall OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId);
			HRESULT __stdcall OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);
			HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject);
			ULONG __stdcall AddRef();
			ULONG __stdcall Release();

		private:
			DeviceManager* deviceManager;
		};

		IPolicyConfig* policyConfig;
		IMMDeviceEnumerator* deviceEnumerator;
		NotificationClient notificationClient = NotificationClient(this);

		const OnChange onChange;
		const OnFatalError onFatalError;

	};
};
