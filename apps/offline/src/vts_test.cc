#include <iostream>
#include <memory>
#include <optional>
#include <thread>

#include "data_io/DataIO.h"
#include "slam/Sensors.h"
#include "Viewer/PangoViewer.h"
#include "slam/VoxelMap.h"

using namespace slam;
slam::TofCamera GetCam();
int main(int argc, char** argv) {
    std::string data_folder = argv[1];
    std::string config_file = "../../config/OrbMachineNarrow.json";

    std::shared_ptr<DataIO::DataSetType::OrbMachineData> data_ptr =
        std::make_shared<DataIO::DataSetType::OrbMachineData>(data_folder);

    PangoViewer viewer;
    viewer.Start();
    cv::namedWindow("front", cv::WINDOW_NORMAL);

    std::vector<cv::Mat> images;
    double image_timestamp;
    slam::TofCamera cam = GetCam();

    VoxelTree::subvoxel_size_ = 0.06;
    VoxelTreeMap map(0.8);

    utils::Transform3d Tob; Tob.SetIdentity();
    Tob.R() << 0.0, -1.0, 0.0,
               0.0, 0.0, -1.0,
               1.0, 0.0, 0.0;
    utils::Transform3d Tro; Tro.SetIdentity();
    Tro.R() << 0.999822, 0.00373944, 0.0185159,
               0.00367504, -0.999987, 0.00351106,
               0.0185288, -0.00344239, -0.999822;
    Tro.t() << 0.214261, -0.117647, 0.000256731;
    Tro.t() = Tro.R() * Tro.t();
    utils::Transform3d To1o2; To1o2.SetIdentity();
    To1o2.R(1, 1) = -1.; To1o2.R(2, 2) = -1.;

    cv::Mat front_image;
    double scale = 0.001;
    utils::Transform3d Trw;
    std::optional<utils::Transform3d> Tbw0 = std::nullopt;
    std::vector<Eigen::Vector3d> pts, colors;
    utils::Transform3d Tbib0;
    while (data_ptr->GetImages(images, image_timestamp)) {
        image_timestamp -= 0.31;
        if (image_timestamp >= 1660903500.589 && image_timestamp < 1660903504.889) continue;
        if (image_timestamp >= 1660903495.136 && image_timestamp < 1660903495.388) continue;

        std::cout.precision(13);
        std::cout << "timestamp: " << image_timestamp << std::endl;
        if (data_ptr->GetVtsPose(Trw, image_timestamp)) {
            utils::Transform3d Tbw = Trw * Tro * Tob.Inverse();
            if (Tbw0 == std::nullopt) Tbw0 = std::make_optional(Tbw);
            Tbib0 = (*Tbw0).Inverse() * Tbw;
            if (Tbib0.t(1) < -0.18) continue;

            // pts.clear(); colors.clear();
            for (size_t i = 0; i < images.size(); i++) {
                cv::Mat depth0;
                images[0].convertTo(depth0, CV_64F);
                depth0 *= scale;
                Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic,
                Eigen::RowMajor>> depths((double*)depth0.data, cam.height, cam.width);

                int bank = 10;
                for (int r = bank; r < cam.height - bank; r++) {
                    for (int c = bank; c < cam.width - bank; c++) {
                        double depth = depth0.at<double>(r, c);// depths(r, c);
                        if (depth < cam.max_depth && depth > cam.min_depth) {
                            Eigen::Vector3d pt = cam.Depth2BodyPt(r, c, depth);
                            Eigen::Vector3d ptw = Tbib0.transform(pt);
                            map.AddPoint(ptw);
                        }
                    }
                }
            }

            // reading global map
            VoxelTreeMap::MapType::iterator iter = map.begin();
            for (; iter != map.end(); iter++) {
                for (size_t i = 0; i < iter->second.Size(); i++) {
                    int N = iter->second.Voxel(i).N;
                    if (N >= 6) {
                        pts.push_back(iter->second.Voxel(i).center);
                        colors.push_back(Eigen::Vector3d(1., 0., 0.));
                    }
                }
            }
            viewer.SetPointCloud(pts, colors, 2);
        }

        Eigen::Matrix4d pose;
        pose.topLeftCorner(3, 3) = Tbib0.R();
        pose.topRightCorner(3, 1) = Tbib0.t();
        viewer.SetOdomPose(pose, Eigen::Vector3f(1, 0, 0));
        viewer.AddPosition(Tbib0.t().cast<float>());
        front_image = images[0].clone();
        front_image *= 0.3;
        front_image.convertTo(front_image, CV_8UC1);
        cv::imshow("front", front_image);
        cv::waitKey(0);
    }

    while (!viewer.ShouldStop()) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

slam::TofCamera GetCam() {
    slam::TofCamera cam;
    cam.Tcb.R() << 1.0, 0.0, 0.0,
                   0.0, 0.996195, -0.0871557,
                   0.0, 0.0871557, 0.996195;
    cam.Tcb.t() << 0.0, -0.01598, 0.1637;
    cam.width = 320;
    cam.height = 240;
    cam.fx = 121.77586;
    cam.fy = 121.6354;
    cam.cx = 160.9986;
    cam.cy = 115.61541;
    cam.fx_inv = 1. / cam.fx;
    cam.fy_inv = 1. / cam.fy;
    cam.max_depth = 6.;
    cam.min_depth = 0.1;
    return cam;
}
