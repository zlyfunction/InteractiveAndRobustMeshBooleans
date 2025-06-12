
#ifdef _MSC_VER // Workaround for known bugs and issues on MSVC
#define _HAS_STD_BYTE \
    0 // https://developercommunity.visualstudio.com/t/error-c2872-byte-ambiguous-symbol/93889
#define NOMINMAX // https://stackoverflow.com/questions/1825904/error-c2589-on-stdnumeric-limitsdoublemin
#endif

#include "booleans.h"

// std::vector<std::string> files;

#include <fstream>
#include <nlohmann/json.hpp>

struct OperationLogData
{
    std::vector<std::vector<int>> T_before;
    std::vector<std::vector<double>> V_before;
    std::vector<std::vector<int>> T_after;
    std::vector<std::vector<double>> V_after;
};

OperationLogData readOperationLog(const std::string& filedir)
{
    OperationLogData data;

    std::ifstream file(filedir);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filedir << std::endl;
        return data;
    }

    nlohmann::json j;
    file >> j;

    // Read T_before
    if (j.contains("T_before") && j["T_before"].contains("values")) {
        for (const auto& row : j["T_before"]["values"]) {
            std::vector<int> t_row;
            for (const auto& val : row) {
                t_row.push_back(val);
            }
            data.T_before.push_back(t_row);
        }
    }

    // Read V_before
    if (j.contains("V_before") && j["V_before"].contains("values")) {
        for (const auto& row : j["V_before"]["values"]) {
            std::vector<double> v_row;
            for (const auto& val : row) {
                v_row.push_back(val);
            }
            data.V_before.push_back(v_row);
        }
    }

    // Read T_after
    if (j.contains("T_after") && j["T_after"].contains("values")) {
        for (const auto& row : j["T_after"]["values"]) {
            std::vector<int> t_row;
            for (const auto& val : row) {
                t_row.push_back(val);
            }
            data.T_after.push_back(t_row);
        }
    }

    // Read V_after
    if (j.contains("V_after") && j["V_after"].contains("values")) {
        for (const auto& row : j["V_after"]["values"]) {
            std::vector<double> v_row;
            for (const auto& val : row) {
                v_row.push_back(val);
            }
            data.V_after.push_back(v_row);
        }
    }

    return data;
}

void printMeshData(
    const std::vector<double>& coords,
    const std::vector<uint>& tris,
    const std::vector<uint>& labels,
    const std::string& title)
{
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "                     " << title << std::endl;
    std::cout << std::string(80, '=') << std::endl;

    // Print coordinates in nx3 format
    std::cout << "\n📍 Coordinates (" << coords.size() / 3 << "x3):" << std::endl;
    for (int i = 0; i < coords.size(); i += 3) {
        std::cout << "  [" << std::setw(3) << i / 3 << "] " << std::fixed << std::setprecision(6)
                  << std::setw(12) << coords[i] << " " << std::setw(12) << coords[i + 1] << " "
                  << std::setw(12) << coords[i + 2] << std::endl;
    }

    // Print triangles in nx3 format
    std::cout << "\n🔻 Triangles (" << tris.size() / 3 << "x3):" << std::endl;
    for (int i = 0; i < tris.size(); i += 3) {
        std::cout << "  [" << std::setw(3) << i / 3 << "] " << std::setw(6) << tris[i] << " "
                  << std::setw(6) << tris[i + 1] << " " << std::setw(6) << tris[i + 2] << std::endl;
    }

    // Print labels
    std::cout << "\n🏷️  Labels (" << labels.size() << "):" << std::endl;
    std::cout << "  ";
    for (int i = 0; i < labels.size(); i++) {
        std::cout << labels[i] << " ";
    }
    std::cout << std::endl;
}

void printOutputMeshData(
    const std::vector<double>& coords,
    const std::vector<uint>& tris,
    const std::vector<std::bitset<NBIT>>& labels,
    const std::string& title)
{
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << title << " (total vertices: " << coords.size() / 3 << ")" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    for (size_t i = 0; i < coords.size(); i += 3) {
        std::cout << std::setw(6) << "v" << std::setw(4) << i / 3 << ": (" << std::fixed
                  << std::setprecision(6) << std::setw(12) << coords[i] << ", " << std::setw(12)
                  << coords[i + 1] << ", " << std::setw(12) << coords[i + 2] << ")" << std::endl;
    }

    // Print triangles with labels
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "TRIANGLES OUTPUT (total triangles: " << tris.size() / 3 << ")" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    for (size_t i = 0; i < tris.size(); i += 3) {
        std::cout << std::setw(6) << "t" << std::setw(4) << i / 3 << ": (" << std::setw(4)
                  << tris[i] << ", " << std::setw(4) << tris[i + 1] << ", " << std::setw(4)
                  << tris[i + 2] << ") → label: " << labels[i / 3] << std::endl;
    }
    std::cout << std::string(60, '=') << "\n" << std::endl;
}


void generateAndSaveMesh(
    FastTrimesh& tm,
    const Labels& labels,
    int label_id,
    const std::string& output_filename,
    bool print_debug = false)
{
    tm.resetTrianglesInfo();
    uint num_tris = 0;

    if (label_id == -1) {
        // All triangles
        for (uint t_id = 0; t_id < tm.numTris(); t_id++) {
            tm.setTriInfo(t_id, 1);
            num_tris++;
        }
    } else {
        // Specific label
        for (uint t_id = 0; t_id < tm.numTris(); t_id++) {
            if (labels.surface[t_id][label_id]) {
                tm.setTriInfo(t_id, 1);
                num_tris++;
            }
        }
    }

    // Prepare output data
    std::vector<double> out_coords;
    std::vector<uint> out_tris;
    std::vector<std::bitset<NBIT>> out_labels;

    // Get the final result
    // computeFinalExplicitResult(tm, labels, num_tris, out_coords, out_tris,
    //                            out_labels, true);
    getFinalMeshInOder(tm, labels, num_tris, out_coords, out_tris, out_labels);

    // Print debug info if requested
    if (print_debug) {
        std::string title = (label_id == -1) ? "ALL TRIANGLES OUTPUT"
                                             : ("LABEL " + std::to_string(label_id) + " OUTPUT");
        printOutputMeshData(out_coords, out_tris, out_labels, title);
    }

    // Write to OBJ file
    cinolib::write_OBJ(output_filename.c_str(), out_coords, out_tris, {});
}

int main(int argc, char** argv)
{
    std::string file_in, file_out;

    if (argc < 3) {
        std::cout << "syntax error!" << std::endl;
        std::cout << "./mesh_booleans_arrangement input.json output.obj" << std::endl;
        return -1;
    }

    file_in = argv[1];
    file_out = argv[2];

    OperationLogData data = readOperationLog(file_in);

    auto T = data.T_before;
    auto V = data.V_before;

    std::vector<double> in_coords, bool_coords;
    std::vector<uint> in_tris, bool_tris;
    std::vector<uint> in_labels;
    std::vector<std::bitset<NBIT>> bool_labels;

    // Convert V to in_coords (flatten the 2D vector)
    for (const auto& vertex : V) {
        for (const auto& coord : vertex) {
            in_coords.push_back(coord);
        }
    }

    // Convert T to in_tris, removing duplicates
    std::set<std::vector<int>> unique_triangles;
    for (const auto& tetrahedron : T) {
        if (tetrahedron.size() >= 4) {
            // Extract the four faces of the tetrahedron
            std::vector<std::vector<int>> faces = {
                {tetrahedron[0], tetrahedron[1], tetrahedron[2]},
                {tetrahedron[0], tetrahedron[1], tetrahedron[3]},
                {tetrahedron[0], tetrahedron[2], tetrahedron[3]},
                {tetrahedron[1], tetrahedron[2], tetrahedron[3]}};

            for (auto& tri : faces) {
                std::sort(tri.begin(), tri.end());
                unique_triangles.insert(tri);
            }
        }
    }

    // Convert unique triangles to in_tris
    for (const auto& triangle : unique_triangles) {
        for (const auto& vertex_id : triangle) {
            in_tris.push_back(static_cast<uint>(vertex_id));
        }
    }

    // Set labels for triangles (assuming all triangles have label 0)
    in_labels.resize(unique_triangles.size(), 0);

    // Randomly sample three points from T to create a label 1 triangle
    std::random_device rd;
    std::mt19937 gen(rd());

    if (!T.empty()) {
        std::uniform_int_distribution<> tet_dis(0, T.size() - 1);
        std::uniform_real_distribution<> bary_dis(0.0, 1.0);

        std::vector<std::vector<double>> sampled_points;

        // Sample three points
        for (int sample = 0; sample < 3; sample++) {
            // Randomly select a tetrahedron
            int tet_id = tet_dis(gen);
            const auto& tet = T[tet_id];

            if (tet.size() >= 4) {
                // Generate random barycentric coordinates
                double r1 = bary_dis(gen);
                double r2 = bary_dis(gen);
                double r3 = bary_dis(gen);
                double r4 = bary_dis(gen);

                // Normalize to ensure they sum to 1
                double sum = r1 + r2 + r3 + r4;
                r1 /= sum;
                r2 /= sum;
                r3 /= sum;
                r4 /= sum;

                // Get vertices of the tetrahedron
                const auto& v0 = V[tet[0]];
                const auto& v1 = V[tet[1]];
                const auto& v2 = V[tet[2]];
                const auto& v3 = V[tet[3]];

                // Interpolate position using barycentric coordinates
                std::vector<double> point(3, 0.0);
                for (int i = 0; i < 3; i++) {
                    point[i] = r1 * v0[i] + r2 * v1[i] + r3 * v2[i] + r4 * v3[i];
                }

                sampled_points.push_back(point);
            }
        }

        // Add sampled points to coordinates and create triangle
        if (sampled_points.size() == 3) {
            uint start_vertex_id = V.size();

            // Add sampled points to in_coords
            for (const auto& point : sampled_points) {
                for (const auto& coord : point) {
                    in_coords.push_back(coord);
                }
            }

            // Add triangle with label 1
            in_tris.insert(
                in_tris.end(),
                {start_vertex_id, start_vertex_id + 1, start_vertex_id + 2});
            in_labels.push_back(1);
        }
    }

    // loadMultipleFiles(files, in_coords, in_tris, in_labels);
    if (false) {
        in_coords = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};

        in_tris = {0, 1, 2, 0, 1, 3, 1, 2, 3, 2, 0, 3};

        in_coords.insert(in_coords.end(), {0.5, -0.2, 0.5, 0.5, 0.8, 0.5, -0.2, 0.3, 0.5});

        in_tris.insert(in_tris.end(), {4, 5, 6});

        // set the labels
        in_labels = {0, 0, 0, 0, 1}; // first 4 faces are the tetrahedron, the last
                                     // face is the intersection triangle
    }

    // Print input data
    printMeshData(in_coords, in_tris, in_labels, "INPUT DATA");

    // init the necessary data structures
    point_arena arena;
    std::vector<genericPoint*> arr_verts;
    std::vector<uint> arr_in_tris, arr_out_tris;
    std::vector<std::bitset<NBIT>> arr_in_labels;
    std::vector<DuplTriInfo> dupl_triangles;
    Labels labels;
    cinolib::Octree octree;

    // arrangement
    customArrangementPipeline(
        in_coords,
        in_tris,
        in_labels,
        arr_in_tris,
        arr_in_labels,
        arena,
        arr_verts,
        arr_out_tris,
        labels,
        octree,
        dupl_triangles,
        false);

    // 创建FastTrimesh
    FastTrimesh tm(arr_verts, arr_out_tris, true);

    // Generate and save meshes using the helper function
    std::string file_out_label0 = file_out.substr(0, file_out.find_last_of('.')) + "_label0.obj";
    std::string file_out_label1 = file_out.substr(0, file_out.find_last_of('.')) + "_label1.obj";

    generateAndSaveMesh(tm, labels, 0, file_out_label0);
    generateAndSaveMesh(tm, labels, 1, file_out_label1);
    generateAndSaveMesh(tm, labels, -1, file_out,
                        true); // Print debug info for all triangles

    bool compute_barycentric = false;
    if (compute_barycentric) {
        // For each triangle in label1, find its containing tetrahedron and
        // barycentric coordinates
        std::ofstream bary_file(
            file_out.substr(0, file_out.find_last_of('.')) + "_label1_barycentric.txt");

        // Get label1 mesh data for barycentric computation
        tm.resetTrianglesInfo();
        uint num_tris_label1 = 0;
        for (uint t_id = 0; t_id < tm.numTris(); t_id++) {
            if (labels.surface[t_id][1]) {
                tm.setTriInfo(t_id, 1);
                num_tris_label1++;
            }
        }

        std::vector<double> out_coords_label1;
        std::vector<uint> out_tris_label1;
        std::vector<std::bitset<NBIT>> out_labels_label1;
        // computeFinalExplicitResult(tm, labels, num_tris_label1,
        // out_coords_label1,
        //  out_tris_label1, out_labels_label1, true);

        // Process triangles directly from the label1 output data
        uint num_output_tris = out_tris_label1.size() / 3;
        for (uint tri_idx = 0; tri_idx < num_output_tris; tri_idx++) {
            std::cout << "Processing triangle " << tri_idx << std::endl;

            // Get the three vertices of this triangle from output data
            uint v0_idx = out_tris_label1[tri_idx * 3 + 0];
            uint v1_idx = out_tris_label1[tri_idx * 3 + 1];
            uint v2_idx = out_tris_label1[tri_idx * 3 + 2];

            // Get coordinates of the three vertices from output coordinates
            cinolib::vec3d p0(
                out_coords_label1[v0_idx * 3 + 0],
                out_coords_label1[v0_idx * 3 + 1],
                out_coords_label1[v0_idx * 3 + 2]);
            cinolib::vec3d p1(
                out_coords_label1[v1_idx * 3 + 0],
                out_coords_label1[v1_idx * 3 + 1],
                out_coords_label1[v1_idx * 3 + 2]);
            cinolib::vec3d p2(
                out_coords_label1[v2_idx * 3 + 0],
                out_coords_label1[v2_idx * 3 + 1],
                out_coords_label1[v2_idx * 3 + 2]);

            // Find which tetrahedron from input T contains this triangle
            bool found_tet = false;
            for (uint tet_id = 0; tet_id < T.size() && !found_tet; tet_id++) {
                if (T[tet_id].size() != 4) continue; // Skip if not a tetrahedron
                std::cout << "checking tet_id: " << tet_id << std::endl;
                // Get the four vertices of the tetrahedron from input T
                uint tet_v0 = T[tet_id][0];
                uint tet_v1 = T[tet_id][1];
                uint tet_v2 = T[tet_id][2];
                uint tet_v3 = T[tet_id][3];

                // Get coordinates from input V
                if (tet_v0 >= V.size() || tet_v1 >= V.size() || tet_v2 >= V.size() ||
                    tet_v3 >= V.size())
                    continue;

                cinolib::vec3d tet_p0(V[tet_v0][0], V[tet_v0][1], V[tet_v0][2]);
                cinolib::vec3d tet_p1(V[tet_v1][0], V[tet_v1][1], V[tet_v1][2]);
                cinolib::vec3d tet_p2(V[tet_v2][0], V[tet_v2][1], V[tet_v2][2]);
                cinolib::vec3d tet_p3(V[tet_v3][0], V[tet_v3][1], V[tet_v3][2]);

                // Check if all three triangle vertices are inside or on this
                // tetrahedron
                // TODO: compute this in rationals/intervals?
                double bary0[4], bary1[4], bary2[4];
                cinolib::tet_barycentric_coords(tet_p0, tet_p1, tet_p2, tet_p3, p0, bary0);
                cinolib::tet_barycentric_coords(tet_p0, tet_p1, tet_p2, tet_p3, p1, bary1);
                cinolib::tet_barycentric_coords(tet_p0, tet_p1, tet_p2, tet_p3, p2, bary2);

                // Set epsilon threshold for cleaning up small values
                double eps = 1e-5;

                // Clean up barycentric coordinates by setting small values to 0
                for (int i = 0; i < 4; i++) {
                    if (std::abs(bary0[i]) < eps) bary0[i] = 0.0;
                    if (std::abs(bary1[i]) < eps) bary1[i] = 0.0;
                    if (std::abs(bary2[i]) < eps) bary2[i] = 0.0;
                }
                // std::cout << "bary0: " << bary0[0] << " " << bary0[1] << " " <<
                // bary0[2]
                //           << " " << bary0[3] << std::endl;
                // std::cout << "bary1: " << bary1[0] << " " << bary1[1] << " " <<
                // bary1[2]
                //           << " " << bary1[3] << std::endl;
                // std::cout << "bary2: " << bary2[0] << " " << bary2[1] << " " <<
                // bary2[2]
                //           << " " << bary2[3] << std::endl;
                // Check if all barycentric coordinates are valid (not inf) and
                // non-negative
                bool inside0 =
                    (bary0[0] != cinolib::inf_double && bary0[1] != cinolib::inf_double &&
                     bary0[2] != cinolib::inf_double && bary0[3] != cinolib::inf_double &&
                     bary0[0] >= 0 && bary0[1] >= 0 && bary0[2] >= 0 && bary0[3] >= 0);
                bool inside1 =
                    (bary1[0] != cinolib::inf_double && bary1[1] != cinolib::inf_double &&
                     bary1[2] != cinolib::inf_double && bary1[3] != cinolib::inf_double &&
                     bary1[0] >= 0 && bary1[1] >= 0 && bary1[2] >= 0 && bary1[3] >= 0);
                bool inside2 =
                    (bary2[0] != cinolib::inf_double && bary2[1] != cinolib::inf_double &&
                     bary2[2] != cinolib::inf_double && bary2[3] != cinolib::inf_double &&
                     bary2[0] >= 0 && bary2[1] >= 0 && bary2[2] >= 0 && bary2[3] >= 0);

                if (inside0 && inside1 && inside2) {
                    // Output the tetrahedron ID and barycentric coordinates
                    bary_file << "Triangle " << tri_idx << " -> Input Tetrahedron " << tet_id
                              << std::endl;
                    bary_file << "  Vertex 0 barycentric: " << bary0[0] << " " << bary0[1] << " "
                              << bary0[2] << " " << bary0[3] << std::endl;
                    bary_file << "  Vertex 1 barycentric: " << bary1[0] << " " << bary1[1] << " "
                              << bary1[2] << " " << bary1[3] << std::endl;
                    bary_file << "  Vertex 2 barycentric: " << bary2[0] << " " << bary2[1] << " "
                              << bary2[2] << " " << bary2[3] << std::endl;
                    found_tet = true;
                }
            }

            if (!found_tet) {
                std::cerr << "Warning: Could not find containing input tetrahedron for "
                             "triangle "
                          << tri_idx << std::endl;
            }
        }

        bary_file.close();
    }
    return 0;
}