#include "TransformInfo.h"
#include "Renderable\ObjMesh.h"
#include "ldpMat\Quaternion.h"
#include <sstream>
namespace ldp
{
	TransformInfo::TransformInfo()
	{
		m_T.zeros();

		// by default, the conversion is from (x,y,0) to (x,0,y)
		m_T.setRotationPart(ldp::QuaternionF().fromAngles(ldp::Float3(ldp::PI_S/2, 0, 0)).toRotationMatrix3());

		// then move foward
		m_T(1, 3) = -0.3; // in meters

		// to handel CCW/CW issues of [triangle]
		m_flipNormal = true;
	}

	TransformInfo::~TransformInfo()
	{
	
	}

	void TransformInfo::flipNormal()
	{
		m_flipNormal = !m_flipNormal;
	}

	void TransformInfo::apply(ObjMesh& mesh)
	{
		if (m_flipNormal)
		{
			for (auto& f : mesh.face_list)
				std::reverse(f.vertex_index, f.vertex_index + f.vertex_count);
		}
		mesh.transform(m_T);
	}

	void TransformInfo::setIdentity()
	{
		m_T.eye();
		m_flipNormal = false;
	}

	void TransformInfo::translate(Float3 t)
	{
		m_T.setTranslationPart(m_T.getTranslationPart() + t);
	}

	void TransformInfo::rotate(Mat3f R, Float3 center)
	{
		m_T.setRotationPart(R*m_T.getRotationPart());
		m_T.setTranslationPart(R * (m_T.getTranslationPart()-center) + center);
	}

	void TransformInfo::scale(Float3 S, Float3 center)
	{
		ldp::Mat3f SM = ldp::Mat3f().eye();
		for (int k = 0; k < 3; k++)
			SM(k, k) = S[k];
		m_T.setRotationPart(SM*m_T.getRotationPart());
		m_T.setTranslationPart(SM * (m_T.getTranslationPart() - center) + center);
	}


	TiXmlElement* TransformInfo::toXML(TiXmlNode* parent)const
	{
		TiXmlElement* ele = new TiXmlElement(getTypeString().c_str());
		parent->LinkEndChild(ele);
		ele->SetAttribute("FlipNormal", m_flipNormal);
		std::string s;
		for (int i = 0; i < m_T.nRow(); i++)
		for (int j = 0; j < m_T.nCol(); j++)
			s += std::to_string(m_T(i, j)) + " ";
		ele->SetAttribute("T", s.c_str());
		return ele;
	}

	void TransformInfo::fromXML(TiXmlElement* self)
	{
		int tmp = 0;
		if (!self->Attribute("FlipNormal", &tmp))
			printf("warning: transformInfo.flipNormal lost");
		m_flipNormal = !!tmp;
		std::string s = self->Attribute("T");
		if (s.empty())
			printf("warning: transformInfo.T lost");
		std::stringstream ts(s);
		m_T.eye();
		for (int i = 0; i < m_T.nRow(); i++)
		for (int j = 0; j < m_T.nCol(); j++)
			ts >> m_T(i, j);
	}
}