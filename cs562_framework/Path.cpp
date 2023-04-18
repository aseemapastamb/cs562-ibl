#include "stdafx.h"

#include "Path.h"

#include "FrameRateManager.h"
#include "Shader.h"

std::vector<glm::vec3> Path::initial_vertices;
std::vector<glm::vec3> Path::control_points;
std::vector<glm::vec3> Path::path_vertices;
std::map<float, std::pair<float, uint32_t>> Path::arc_length;
size_t Path::num_segments = 0;
size_t Path::num_entries_per_seg = 0;
float Path::t1 = 0.0f;
float Path::t2 = 0.0f;
float Path::t_total = 0.0f;
float Path::vc = 0.0f;
float Path::t_curr = 0.0f;
bool Path::path_completed = false;
glm::vec3 Path::pos = glm::vec3{ 0.0f, 0.0f, 0.0f };
glm::vec3 Path::pitch = glm::vec3{ 0.0f, 0.0f, 0.0f };
glm::vec3 Path::yaw = glm::vec3{ 0.0f, 0.0f, 0.0f };
glm::vec3 Path::roll = glm::vec3{ 0.0f, 0.0f, 0.0f };

void Path::SetupDefaultPath() {
    std::vector<glm::vec3> default_path = {
    glm::vec3{ 8.0f, 0.0f, 0.0f },

    glm::vec3{ 6.0f, 0.0f, -3.0f },     // segments will exist between these points
    glm::vec3{ 2.0f, 0.0f, -3.0f },
    glm::vec3{ -1.0f, 0.0f, -1.0f },
    glm::vec3{ 0.0f, 0.0f, 0.0f },
    glm::vec3{ -1.0f, 0.0f, 2.0f },
    glm::vec3{ 1.0f, 0.0f, 4.0f },
    glm::vec3{ 2.0f, 0.0f, 4.5f },
    glm::vec3{ 4.0f, 0.0f, 6.0f },

    glm::vec3{ 8.0f, 0.0f, 6.0f }       // first and last points used as control points only
    };
    SetupPath(default_path);
}

void Path::GeneratePath(glm::vec3 model_pos, glm::vec3 target_pos) {
    t_curr = 0.0f;

    target_pos.y = 0.0f;
    glm::vec3 direction = target_pos - model_pos;
    direction = glm::normalize(direction);

    float total_dist = glm::distance(model_pos, target_pos);
    float seg_dist = total_dist / 8.0f;

    std::vector<glm::vec3> new_path;
    // first point - just behind model pos
    new_path.push_back(glm::vec3{ model_pos + (direction * (-seg_dist)) });

    // intermediate points
    for (uint32_t i = 0; i <= 8; ++i) {
        new_path.push_back(glm::vec3{ model_pos + (direction * (i * seg_dist)) });
    }

    // last point - just beyond target pos
    //new_path.push_back(glm::vec3{ target_pos + (direction * seg_dist) });

    SetupPath(new_path);
}

void Path::SetupPath(std::vector<glm::vec3> points) {
    initial_vertices.clear();
    initial_vertices = points;
    control_points.clear();
    path_vertices.clear();

    // insert more control points to smooth path along keyframes
    // this is done to ensure C1 continuity
    size_t path_size = initial_vertices.size();
    for (uint32_t i = 1; i < path_size - 1; ++i) {
        // p = 2pi - pi-1
        glm::vec3 p = initial_vertices[i] + initial_vertices[i] - initial_vertices[i - 1];
        glm::vec3 ai = (p + initial_vertices[i + 1]) / 2.0f;
        glm::vec3 bi = initial_vertices[i] + initial_vertices[i] - ai;

        control_points.push_back(bi);
        control_points.push_back(initial_vertices[i]);
        control_points.push_back(ai);
    }
    num_segments = path_size - 3;

    // Cubic Bezier curve interpolation
    // adding 10 points in each segment for a smooth curve
    size_t new_path_size = control_points.size();
    for (uint32_t i = 1; i < new_path_size - 3; i = i + 3) {
        for (uint32_t j = 0; j < 10; ++j) {
            float u = j / 9.0f;
            std::vector<glm::vec3> seg_control_pts;
            seg_control_pts.push_back(control_points[i]);
            seg_control_pts.push_back(control_points[i + 1]);
            seg_control_pts.push_back(control_points[i + 2]);
            seg_control_pts.push_back(control_points[i + 3]);
            glm::vec3 vert = Bezier(u, seg_control_pts);
            path_vertices.push_back(vert);
        }
    }

    // arc length table generation
    std::vector<std::map<float, float>> arc_length_tables;

    // loop through set of 3 values (b_i-1, p_i, a_i)
    for (uint32_t i = 1; i < new_path_size - 3; i = i + 3) {
        std::map<float, float> segment_arc_lengths;
        uint32_t index = 0;
        segment_arc_lengths.emplace(0.0f, 0.0f);
        std::vector<glm::vec2> segment_list;
        segment_list.push_back(glm::vec2{ 0.0f, 1.0f });
        while (!segment_list.empty()) {
            std::vector<glm::vec3> control_pts;
            control_pts.push_back(control_points[i]);       // p_i
            control_pts.push_back(control_points[i + 1]);   // a_i
            control_pts.push_back(control_points[i + 2]);   // b_i
            control_pts.push_back(control_points[i + 3]);   // p_i+1

            glm::vec2 segment = segment_list.back();
            segment_list.pop_back();                        // remove [ua, ub]

            float ua = segment.x;
            float ub = segment.y;
            float um = (ua + ub) / 2.0f;

            float A = glm::length(Bezier(ua, control_pts) - Bezier(um, control_pts));
            float B = glm::length(Bezier(um, control_pts) - Bezier(ub, control_pts));
            float C = glm::length(Bezier(ua, control_pts) - Bezier(ub, control_pts));
            float d = A + B - C;

            if ((d > 0.01f) || (fabsf(ua - ub) > 0.01f)) {
                // insert [ua, um] and [um, ub]
                segment_list.push_back(glm::vec2{ um, ub });
                segment_list.push_back(glm::vec2{ ua, um });
            }
            else {
                // record values in arc length table for this segment
                float sm = segment_arc_lengths.at(ua) + A;
                float sb = sm + B;
                segment_arc_lengths.emplace(um, sm);
                segment_arc_lengths.emplace(ub, sb);
            }
        }
        // add arc length table for this segment to collection of all arc length tables
        arc_length_tables.push_back(segment_arc_lengths);
    }

    // normalize arc lengths over curve
    std::map<float, std::pair<float, uint32_t>> arc_lengths_non_norm;
    size_t arc_lengths_size = arc_length_tables.size();
    for (uint32_t i = 0; i < arc_lengths_size; ++i) {
        if (i == 0) {
            for (auto& j : arc_length_tables[i]) {
                arc_lengths_non_norm.emplace(j.first, std::make_pair(j.second, i));
            }
        }
        else {
            for (auto& j : arc_length_tables[i]) {
                if (j.first != 0.0f) {
                    // add u and s values of previous table
                    float u_temp = j.first + static_cast<float>(i);
                    float s_temp = j.second + arc_lengths_non_norm.at(static_cast<float>(i)).first;
                    arc_lengths_non_norm.emplace(u_temp, std::make_pair(s_temp, i));
                }
            }
        }
    }
    float u_max = arc_lengths_non_norm.rbegin()->first;
    float s_max = arc_lengths_non_norm.rbegin()->second.first;
    for (auto& i : arc_lengths_non_norm) {
        float key = i.first / u_max;
        float val = i.second.first / s_max;
        arc_length.emplace(key, std::make_pair(val, i.second.second));
    }
    num_entries_per_seg = arc_length.size() / num_segments;
}

void Path::SetupSpeed(float _t_total, float _t1, float _t2) {
    // set total time to traverse path
    t_total = _t_total;
    // time ratios for speed up and slow down
    t1 = _t1 / t_total;
    t2 = _t2 / t_total;

    // max speed
    vc = 2.0f / (1.0f + t2 - t1);
}

void Path::DrawPath(Shader* shader) {
    glEnable(GL_LINE_SMOOTH);
    uint32_t path_vao, path_vbo;

    // initial points
    shader->SetVec3("aColor", glm::vec3{ 0.8f, 0.1f, 0.1f });
    glPointSize(20.0f);

    glGenVertexArrays(1, &path_vao);
    glGenBuffers(1, &path_vbo);
    glBindVertexArray(path_vao);
    glBindBuffer(GL_ARRAY_BUFFER, path_vbo);

    glBufferData(GL_ARRAY_BUFFER, (initial_vertices.size() - 2) * sizeof(glm::vec3), &(*(initial_vertices.begin() + 1)), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(initial_vertices.size() - 2));

    glBindVertexArray(0);
    glDeleteBuffers(1, &path_vbo);
    glDeleteVertexArrays(1, &path_vao);

    // final curve
    shader->SetVec3("aColor", glm::vec3{ 0.1f, 0.1f, 0.8f });
    glLineWidth(5.0f);

    glGenVertexArrays(1, &path_vao);
    glGenBuffers(1, &path_vbo);
    glBindVertexArray(path_vao);
    glBindBuffer(GL_ARRAY_BUFFER, path_vbo);

    for (uint32_t i = 0; i < path_vertices.size() - 1; ++i) {
        float p0[6]{
            path_vertices[i].x,
            path_vertices[i].y,
            path_vertices[i].z,
            path_vertices[i + 1].x,
            path_vertices[i + 1].y,
            path_vertices[i + 1].z
        };
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), &p0[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glDrawArrays(GL_LINES, 0, 2);
    }

    glBindVertexArray(0);
    glDeleteBuffers(1, &path_vbo);
    glDeleteVertexArrays(1, &path_vao);

    glDisable(GL_LINE_SMOOTH);
}

// tuple of pos, pitch, yaw, roll
std::tuple<glm::vec3, glm::vec3, glm::vec3, glm::vec3> Path::UpdateModelAlongPath() {
    // add time
    t_curr += p_frame_rate_manager->DeltaTime();
    float t_ratio = t_curr / t_total;

    if (t_ratio < 1.0f) {
        path_completed = false;

        // skidding control
        {
            //// velocity
            //float vel_curr = VelocityTime(t_ratio);

            //// lowest num of intermediate keyframes (occurs at fastest speed)
            //float lowest_num_incr = 1;

            //// current num of intermediate keyframes - based on current velocity
            //// lowest_num_incr when vel_curr = 0 and 30 when vel_curr = vc
            //float num_incr = (lowest_num_incr - ((30 / vc) * vel_curr)) + 30;

            //// update num_incr for each bone
            //std::vector<Bone>& bones = our_animator->curr_anim->bones;
            //uint32_t bones_size = bones.size();
            //for (uint32_t i = 0; i < bones_size; ++i) {
            //    bones[i].SetNumIncr(num_incr);
            //}
        }

        // distance covered along path (arc length)
        float dist_covered = DistanceTime(t_ratio);

        // parameter u and segment index
        std::pair<float, uint32_t> u_i = InverseArcLength(dist_covered);
        float u = u_i.first;
        uint32_t index = u_i.second;

        // position along curve
        u *= arc_length.size();
        u = (u - (num_entries_per_seg * index)) / num_entries_per_seg;

        std::vector<glm::vec3> segment_control_pts;
        segment_control_pts.push_back(control_points[(index * 3) + 1]);
        segment_control_pts.push_back(control_points[(index * 3) + 2]);
        segment_control_pts.push_back(control_points[(index * 3) + 3]);
        segment_control_pts.push_back(control_points[(index * 3) + 4]);

        // set model at this position
        pos = Bezier(u, segment_control_pts);

        // orientation control
        float delta_u = 0.001f;
        glm::vec3 coi = (Bezier(u + delta_u, segment_control_pts) + Bezier(u + (2.0f * delta_u), segment_control_pts) + Bezier(u + (3.0f * delta_u), segment_control_pts)) / 3.0f;
        roll = glm::normalize(coi - pos);
        pitch = glm::normalize(glm::cross(glm::vec3{ 0.0f, 1.0f, 0.0f }, roll));
        yaw = glm::normalize(glm::cross(roll, pitch));
    }
    // end of path
    else {
        path_completed = true;
    }

    return std::make_tuple(pos, pitch, yaw, roll);
}

bool Path::PathCompleted() {
    return path_completed;
}

// HELPER FUNCTIONS

glm::vec3 Path::Bezier(float u, std::vector<glm::vec3> const& control_pts) {
    // geometric form
    return
        (-powf(u, 3) + (3.0f * powf(u, 2)) - (3.0f * u) + 1.0f) * control_pts[0] +
        ((3.0f * powf(u, 3)) - (6.0f * powf(u, 2)) + (3.0f * u)) * control_pts[1] +
        ((-3.0f * powf(u, 3)) + (3.0f * powf(u, 2))) * control_pts[2] +
        (powf(u, 3)) * control_pts[3];

    // algebraic form
    //return
    //    (-control_pts[0] + (3.0f * control_pts[1]) - (3.0f * control_pts[2]) + control_pts[3]) * powf(u, 3) +
    //    ((3.0f * control_pts[0]) + (-6.0f * control_pts[1]) + (3.0f * control_pts[2])) * powf(u, 2) +
    //    ((-3.0f * control_pts[0]) + (3.0f * control_pts[1])) * u +
    //    control_pts[0];
}

// return s value along with index of segment
std::pair<float, uint32_t> Path::ArcLength(float u) {
    float u_prev = 0.0f;
    float s_prev = 0.0f;
    uint32_t index = 0;
    float res = 0.0f;
    for (auto const& i : arc_length) {
        index = i.second.second;
        if (i.first < u) {
            u_prev = i.first;
            s_prev = i.second.first;
        }
        else {
            float k = (u - u_prev) / (i.first - u_prev);
            res = (k * i.second.first) + ((1.0f - k) * s_prev);
            break;
        }
    }
    return std::make_pair(res, index);
}

// return u value along with index of segment
std::pair<float, uint32_t> Path::InverseArcLength(float s) {
    float ua = 0.0f;
    float ub = 1.0f;
    float um = 0.0f;
    std::pair<float, uint32_t> sm_i = std::make_pair(0.0f, 0);
    while (fabsf(s - sm_i.first) > 0.001f) {
        um = (ua + ub) / 2.0f;
        sm_i = ArcLength(um);
        if (sm_i.first < s) {
            ua = um;
        }
        else {
            ub = um;
        }
    }
    return std::make_pair(um, sm_i.second);
}

float Path::DistanceTime(float t) {
    float s = 0.0f;
    // ease in
    if (t > 0.0f && t <= t1) {
        s = (vc * t * t) / (2.0f * t1);
    }
    // constant speed
    if (t > t1 && t <= t2) {
        s = vc * (t - (t1 / 2.0f));
    }
    // ease out
    if (t > t2 && t <= 1.0f) {
        s = ((vc * (t - t2) * (2.0f - t - t2)) / (2.0f * (1.0f - t2))) + (vc * (t2 - (t1 / 2.0f)));
    }
    return s;
}

float Path::VelocityTime(float t) {
    float v = 0.0f;
    // ease in
    if (t > 0.0f && t <= t1) {
        v = (vc * t) / t1;
    }
    // constant speed
    if (t > t1 && t <= t2) {
        v = vc;
    }
    // ease out
    if (t > t2 && t <= 1.0f) {
        v = (vc * (1.0f - t)) / (1.0f - t2);
    }
    return v;
}