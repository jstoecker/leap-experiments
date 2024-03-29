#include "PoseExperiment.h"
#include "gl/geom/Plane.h"
#include "gl/geom/Sphere.h"
#include "util/stb_image.h"

using namespace gl;
using namespace std;
using namespace std::chrono;
using namespace Leap;

PoseExperiment::PoseExperiment(int trials_per_pose) :
    Experiment(trials_per_pose * num_poses),
    valid_pose_(false),
    done_pose_(false),
    trials_per_pose_(trials_per_pose),
    breaks_(0)
{
	cam_control_.camera().radius(7.0f);
    cam_control_.camera().pitch(45.0f * deg_to_rad);
	initTrial();
    
    poses_.enableAll(true);
}

void PoseExperiment::saveTrial()
{
    cout << "trial       : " << (trialsCompleted() + 1) << endl;
	cout << "pose        : " << pose_name_ << endl;
    cout << "breaks      : " << breaks_ << endl;
	cout << "time (ms)   : " << trialTime().count() << endl;
	for (string& s : extra_info_) {
		cout << s << endl;
	}
}

void PoseExperiment::initTrial()
{
    done_pose_ = false;
    valid_pose_ = false;
    breaks_ = 0;
	extra_info_.clear();
}

void PoseExperiment::leapInput(const Leap::Frame& frame)
{
    frame_ = frame;
    updatePose();
}

Texture loadTexImage(const char* filename)
{
	Texture t;
	int width, height, channels;
	unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
	if (!data) {
		cerr << "WARNING: couldn't load " << filename << endl;
	} else {
		t.generate(GL_TEXTURE_2D);
		t.bind();
		t.setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		t.setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		t.setData2D(GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	stbi_image_free(data);
	return t;
}

void PoseExperiment::draw(const gl::Viewport& viewport)
{
    if (!prog_.id()) {
        Geometry geom = Sphere({ 0, 0, 0 }, .075f).triangles(8);
        joint_vbo_.generateVBO(GL_STATIC_DRAW);
        joint_vbo_.bind();
        joint_vbo_.data(&geom.vertices[0], geom.vertices.size() * sizeof(Vec3));
        joint_ibo_.generateIBO(GL_STATIC_DRAW);
        joint_ibo_.bind();
        joint_ibo_.data(&geom.indices[0], geom.indices.size() * sizeof(GLuint));
        joint_triangle_count_ = geom.indices.size();
        
        prog_ = Program::create("shaders/color.vert", "shaders/color.frag");
        text_.loadFont("menlo18");

		// textures

		//{
		//	L_OPEN,
		//		L_CLOSED,
		//		V_OPEN,
		//		V_CLOSED,
		//		CARRY,
		//		FIST_CLOSED,
		//		FIST_THUMB,
		//		FIST_THREE,
		//		PALMS_FACE,
		//		PINCH,
		//		POINT,
		//		POINT2,
		//		PUSH,
		pose_textures_.push_back(loadTexImage("images/l_open.jpg"));
		pose_textures_.push_back(loadTexImage("images/l_closed.jpg"));
		pose_textures_.push_back(loadTexImage("images/v_open.jpg"));
		pose_textures_.push_back(loadTexImage("images/v_closed.jpg"));
		pose_textures_.push_back(loadTexImage("images/carry.jpg"));
		pose_textures_.push_back(loadTexImage("images/fist.jpg"));
		pose_textures_.push_back(loadTexImage("images/thumb.jpg"));
		pose_textures_.push_back(loadTexImage("images/three.jpg"));
		pose_textures_.push_back(loadTexImage("images/palms_face.jpg"));
		pose_textures_.push_back(loadTexImage("images/pinch.jpg"));
		pose_textures_.push_back(loadTexImage("images/point.jpg"));
		pose_textures_.push_back(loadTexImage("images/point2.jpg"));
		//pose_textures_.push_back(loadTexImage("images/push.jpg"));

		GLfloat vertices[] = {
			-0.5f, -0.5f, 0.0f, 0.0f,
			+0.5f, -0.5f, 1.0f, 0.0f,
			+0.5f, +0.5f, 1.0f, -1.0f,
			-0.5f, -0.5f, 0.0f, 0.0f,
			+0.5f, +0.5f, 1.0f, -1.0f,
			-0.5f, +0.5f, 0.0f, -1.0f
		};
		quad_.generateVBO(GL_STATIC_DRAW);
		quad_.bind();
		quad_.data(vertices, sizeof(vertices));

		tex_prog_ = Program::create("shaders/texture_2D.vert", "shaders/texture_2D.frag");
    }
    
	Camera& camera = cam_control_.camera();
	camera.aspect(viewport.aspect());
    view_projection_ = camera.projection() * camera.view();

	// grid plane
	drawing_.color(.75f, .75f, .75f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawing_.begin(GL_TRIANGLES);
	drawing_.setModelViewProj(camera.projection() * camera.view() * scale(15, 15, 15));
	drawing_.geometry(Plane(Vec3::yAxis(), Vec3()).triangles(16, 16));
	drawing_.end();
	drawing_.draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    for (const Hand& hand : frame_.hands()) {
        drawHand(hand);
    }

	tex_prog_.enable();
	tex_prog_.uniform("mvp", viewport.orthoProjection() * translation(viewport.width - 150, viewport.height - 150, 0) * scale(200, 200, 1));
	quad_.bind();
	pose_textures_[(trialsCompleted() / trials_per_pose_)].bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glDrawArrays(GL_TRIANGLES, 0, 6);
    
    text_.color(0, 0, 0);
    text_.hAlign(TextRenderer::HAlign::center);
    text_.vAlign(TextRenderer::VAlign::top);
    text_.viewport(viewport);
    text_.clear();
    text_.add("Pose: " + pose_name_, viewport.width - 150, viewport.height - 25);
    text_.draw();
}

void PoseExperiment::drawHand(const Hand& hand)
{
    Vec4 base_color = (valid_pose_) ? Vec4{0.8f, 0.8f, 0.4f, 1.0f} : Vec4{0.8f, 0.4f, 0.4f, 1.0f};
    
    if (done_pose_) {
        base_color = Vec4(0.4f, 0.8f, 0.4f, 1.0f);
    }
    
	prog_.enable();
	joint_vbo_.bind();
	joint_ibo_.bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    
	prog_.uniform("color", base_color);

    
	for (Finger f : hand.fingers()) {
        
		Vec4 finger_color = base_color;
		if (!f.isExtended())
			finger_color *= 1.5f;
        
		prog_.uniform("color", finger_color);
        
		drawJoint(f.bone(Bone::TYPE_METACARPAL).prevJoint());
		drawJoint(f.bone(Bone::TYPE_PROXIMAL).prevJoint());
		drawJoint(f.bone(Bone::TYPE_INTERMEDIATE).prevJoint());
		drawJoint(f.bone(Bone::TYPE_DISTAL).prevJoint());
        
		drawJoint(f.tipPosition());
	}
    	drawJoint(hand.palmPosition(), 2.0f);
    
	drawing_.setModelViewProj(view_projection_);
	drawing_.color(base_color.x, base_color.y, base_color.z);
	for (Finger f : hand.fingers()) {
        
		if (f.type() == Finger::TYPE_INDEX || f.type() == Finger::TYPE_PINKY) {
			drawLine(f.bone(Bone::TYPE_METACARPAL).prevJoint(), f.bone(Bone::TYPE_METACARPAL).nextJoint());
		}
		drawLine(f.bone(Bone::TYPE_PROXIMAL).prevJoint(), f.bone(Bone::TYPE_PROXIMAL).nextJoint());
		drawLine(f.bone(Bone::TYPE_INTERMEDIATE).prevJoint(), f.bone(Bone::TYPE_INTERMEDIATE).nextJoint());
		drawLine(f.bone(Bone::TYPE_DISTAL).prevJoint(), f.bone(Bone::TYPE_DISTAL).nextJoint());
        
	}
    
	drawLine(hand.fingers()[1].bone(Bone::TYPE_PROXIMAL).prevJoint(), hand.fingers()[2].bone(Bone::TYPE_PROXIMAL).prevJoint());
	drawLine(hand.fingers()[2].bone(Bone::TYPE_PROXIMAL).prevJoint(), hand.fingers()[3].bone(Bone::TYPE_PROXIMAL).prevJoint());
	drawLine(hand.fingers()[3].bone(Bone::TYPE_PROXIMAL).prevJoint(), hand.fingers()[4].bone(Bone::TYPE_PROXIMAL).prevJoint());
	drawLine(hand.fingers()[1].bone(Bone::TYPE_METACARPAL).prevJoint(), hand.fingers()[2].bone(Bone::TYPE_METACARPAL).prevJoint());
	drawLine(hand.fingers()[2].bone(Bone::TYPE_METACARPAL).prevJoint(), hand.fingers()[3].bone(Bone::TYPE_METACARPAL).prevJoint());
	drawLine(hand.fingers()[3].bone(Bone::TYPE_METACARPAL).prevJoint(), hand.fingers()[4].bone(Bone::TYPE_METACARPAL).prevJoint());
}

void PoseExperiment::drawJoint(Vector center, float scale)
{
	Mat4 model = translation(convert(center)) * gl::scale(scale, scale, scale);
	prog_.uniform("modelViewProjection", view_projection_ * model);
	glDrawElements(GL_TRIANGLES, joint_triangle_count_, GL_UNSIGNED_INT, 0);
}

void PoseExperiment::drawLine(Vector start, Vector end)
{
	Vec3 a = convert(start);
	Vec3 b = convert(end);
	drawing_.begin(GL_LINES);
	drawing_.vertex(a.x, a.y, a.z);
	drawing_.vertex(b.x, b.y, b.z);
	drawing_.end();
	drawing_.draw();
}

Vec3 PoseExperiment::convert(const Vector& v)
{
	Vec3 result = v.toVector3<Vec3>();
	return result * 0.015f;
}

void PoseExperiment::updatePose()
{
    poses_.update(frame_);
    
    bool was_valid = valid_pose_;
    
    if (!trialInProgress() && frame_.hands().count() > 0) {
        startTrial();
    }
    
    PoseType pose = (PoseType)(trialsCompleted() / trials_per_pose_);
    switch(pose)
    {
        case PoseType::V_OPEN:
            pose_name_ = "V - Open";
            valid_pose_ =  poses_.v().tracking() && !poses_.v().isClosed();
            break;
        case PoseType::V_CLOSED:
            pose_name_ = "V - Closed";
            valid_pose_ =  poses_.v().tracking() && poses_.v().isClosed();
            break;
        case PoseType::L_OPEN:
            pose_name_ = "L - Open";
            valid_pose_ =  poses_.l().tracking() && !poses_.l().isClosed();
            break;
        case PoseType::L_CLOSED:
            pose_name_ = "L - Closed";
            valid_pose_ =  poses_.l().tracking() && poses_.l().isClosed();
            break;
        case PoseType::CARRY:
            pose_name_ = "Carry";
            valid_pose_ = poses_.carry().tracking();
            break;
        case PoseType::FIST_CLOSED:
            pose_name_ = "Fist - Closed";
            valid_pose_ = poses_.fist().tracking() && poses_.fist().state() == FistPose::State::closed;
            break;
        case PoseType::FIST_THUMB:
            pose_name_ = "Fist - Thumb Out";
            valid_pose_ = poses_.fist().tracking() && poses_.fist().state() == FistPose::State::thumb_out;
            break;
        case PoseType::FIST_THREE:
            pose_name_ = "Fist - Three Extended";
            valid_pose_ = poses_.fist().tracking() && poses_.fist().state() == FistPose::State::three_out;
            break;
        case PoseType::PALMS_FACE:
            pose_name_ = "Palms Face";
            valid_pose_ = poses_.palmsFace().tracking();
            break;
        case PoseType::PINCH:
            pose_name_ = "Pinch";
            valid_pose_ = poses_.pinch().tracking() && poses_.pinch().isPinching();
            break;
        case PoseType::POINT:
            pose_name_ = "Point";
            valid_pose_ = poses_.point().tracking();
            break;
        case PoseType::POINT2:
            pose_name_ = "Point - Two Hands";
            valid_pose_ = poses_.point2().tracking();
            break;
        // case PoseType::PUSH:
        //     pose_name_ = "Push";
        //     valid_pose_ = poses_.push().tracking();
            // break;
        default:
            valid_pose_ = false;
    }
    
    if (done_pose_ && !valid_pose_) {
        stopTrial();
    }
    
    if (!done_pose_ && valid_pose_ && !was_valid) {
        start_valid_ = high_resolution_clock::now();
    }
    
    if (trialInProgress() && !done_pose_ && !valid_pose_ && was_valid) {
        breaks_++;
    }
    
    if (valid_pose_ && was_valid) {
        milliseconds elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - start_valid_);
        if (elapsed.count() > 500) {
			bool was_done = done_pose_;
            done_pose_ = true;

			if (!was_done) {
				std::ostringstream ss;
				switch (pose)
				{
				case PoseType::V_CLOSED:
				case PoseType::V_OPEN:
					ss << "separation  : " << poses_.v().separation();
					extra_info_.push_back(ss.str());
					break;
				case PoseType::L_CLOSED:
				case PoseType::L_OPEN:
					ss << "separation  : " << poses_.l().separation();
					extra_info_.push_back(ss.str());
					break;
				}
			}
        }
    }
    
}