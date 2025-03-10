
#pragma once

#include "framework.h"

#include <functional>
#include <optional>
#include <vector>
#include "Audio.h"
#include "Device.h"

namespace Audio
{
	class DeviceManager final
	{
	public:
		using OnChange = std::function<void()>;
		using OnFatalError = std::function<void(std::wstring message)>;

		/**
		 * @param onChange Callback upon something about the device list changing. This is lazy, but given the scope of this app, works.
		 * @param onFatalError Callback upon an unrecoverable error occurring. Again, using this due to the small scope of this application - COM failures aren't really that "recoverable" I would imagine, since this app has only one real function anyway.
		 */
		DeviceManager(const OnChange& onChange, const OnFatalError& onFatalError);

		~DeviceManager();

		/**
		 * @brief Refreshes the list of audio devices.
		 * @return An error, if one occurred during the operation.
		 */
		[[nodiscard]]
		std::optional<Error> refresh();

		/**
		 * @brief Gets an audio device by its unique ID.
		 * @param id Opaque identifier of the audio device.
		 * @return The audio device associated with the provided ID.
		 */
		[[nodiscard]]
		const Device& operator[](Device::Id& id) const;

		/**
		 * @brief Gets an audio device by its unique ID.
		 * @param id Opaque identifier of the audio device.
		 * @return The audio device associated with the provided ID.
		 */
		[[nodiscard]]
		Device& operator[](Device::Id& id);

		/**
		 * @brief Gets the audio device that's been set as the default.
		 * @return The default audio device.
		 */
		[[nodiscard]]
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
		std::optional<Error> setDefault(Device::Id& id);

		std::vector<Device> devices;

	private:
		/**
		 * @brief Client to notify us of any changes to audio devices,
		 * we wrap this into a nicer interface for the consumer that uses our `Device` type.
		 */
		class NotificationClient final : public IMMNotificationClient
		{
		public:
			explicit NotificationClient(DeviceManager* deviceManager)
				: deviceManager(deviceManager) {}

			HRESULT __stdcall OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;

			HRESULT __stdcall OnDeviceAdded(LPCWSTR pwstrDeviceId) override;

			HRESULT __stdcall OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;

			HRESULT __stdcall OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;

			HRESULT __stdcall OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override;

			HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;

			ULONG __stdcall AddRef() override;

			ULONG __stdcall Release() override;

		private:
			DeviceManager* deviceManager;
			ULONG refCount = 1;
		};

		IPolicyConfig* policyConfig;
		IMMDeviceEnumerator* deviceEnumerator;
		IMMNotificationClient* notificationClient = new NotificationClient(this);

		const OnChange onChange;
		const OnFatalError onFatalError;
	};
};
