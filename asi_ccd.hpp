/*
    ASI CCD Driver

    Copyright (C) 2015-2021 Jasem Mutlaq (mutlaqja@ikarustech.com)
    Copyright (C) 2018 Leonard Bottleman (leonard@whiteweasel.net)
    Copyright (C) 2021 Pawel Soja (kernel32.pl@gmail.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <spdlog/spdlog.h>
#include <libasi/ASICamera2.h>
#include "asi_base.hpp"

#include <map>

# define _ASIGetNumOfConnectedCameras ASIGetNumOfConnectedCameras
# define _ASIGetCameraProperty ASIGetCameraProperty

class ASICCD : public ASIBase
{
    public:
        explicit ASICCD(const ASI_CAMERA_INFO &camInfo, const std::string &cameraName) : ASIBase(camInfo) { 
          mCameraName = cameraName;

        };
};
static class Loader
{
    public:
        std::map<int, std::shared_ptr<ASICCD>> cameras;
        Loader()
        {
            spdlog::debug(__func__);
            load();
        }

    public:
        static size_t getCountOfConnectedCameras()
        {
            return size_t(std::max(_ASIGetNumOfConnectedCameras(), 0));
        }

        static std::vector<ASI_CAMERA_INFO> getConnectedCameras()
        {
            std::vector<ASI_CAMERA_INFO> result(getCountOfConnectedCameras());
            spdlog::info("Detected {} ZWO cameras", result.size());
            int i = 0;
            for(auto &cameraInfo : result)
                _ASIGetCameraProperty(&cameraInfo, i++);
            return result;
        }

    public:
        void load()
        {
            spdlog::debug(__func__);
            auto usedCameras = std::move(cameras);

            UniqueName uniqueName(usedCameras);

            for(const auto &cameraInfo : getConnectedCameras())
            {
                int id = cameraInfo.CameraID;

                // camera already created
                if (usedCameras.find(id) != usedCameras.end())
                {
                    std::swap(cameras[id], usedCameras[id]);
                    continue;
                }

                auto name = uniqueName.make(cameraInfo);
                ASICCD *asiCcd = new ASICCD(cameraInfo, name);
                cameras[id] = std::shared_ptr<ASICCD>(asiCcd);
                spdlog::info("Camera ID: {}; Name: {}", id, name);
            }
        }

    public:
        class UniqueName
        {
                std::map<std::string, bool> used;
            public:
                UniqueName() = default;
                UniqueName(const std::map<int, std::shared_ptr<ASICCD>> &usedCameras)
                {
                    for (const auto &camera : usedCameras)
                        used[camera.second->getDevName()] = true;
                }

                std::string make(const ASI_CAMERA_INFO &cameraInfo)
                {
                    std::string cameraName = "ZWO CCD " + std::string(cameraInfo.Name + 4);
                    std::string uniqueName = cameraName;

                    for (int index = 0; used[uniqueName] == true; )
                        uniqueName = cameraName + " " + std::to_string(++index);

                    used[uniqueName] = true;
                    return uniqueName;
                }
        };
} loader;

