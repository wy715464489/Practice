//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Draw collision implementation
// 
//*****************************************************************************

#include "VuDynamicsDrawCollisionImpl.h"
#include "VuDynamicsImpl.h"
#include "VuEngine/Assets/VuCollisionMeshAsset.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Shaders/VuCollisionShader.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Math/VuPackedVector.h"


namespace VuDynamicsDrawCollisionImpl
{
	class DebugDrawcallback : public btTriangleCallback, public btInternalTriangleIndexCallback
	{
	public:
		DebugDrawcallback() : mVerts(32*3) {}

		virtual void internalProcessTriangleIndex(btVector3* triangle,int partId,int  triangleIndex)
		{
			processTriangle(triangle,partId,triangleIndex);
		}

		virtual void processTriangle(btVector3* triangle,int partId, int triangleIndex)
		{
			mVerts.push_back(triangle[0]);
			mVerts.push_back(triangle[1]);
			mVerts.push_back(triangle[2]);
		}

		void draw(const VuMatrix &transform, const VuColor &color);

		VuArray<btVector3> mVerts;
	};

	static DebugDrawcallback sDebugDrawcallback;
}


//*****************************************************************************
void VuDynamicsDrawCollisionImpl::drawCollision(const VuCamera &camera)
{
	if ( VuGfxUtil::IF()->collisionShader()->getMaterial() == VUNULL )
		return;

	const btCollisionObjectArray &collisionObjects = VuDynamics::IF()->getDynamicsWorld()->getCollisionObjectArray();
	for ( int i = 0; i < collisionObjects.size(); i++ )
	{
		const btCollisionObject *colObj = collisionObjects[i];
		if ( colObj->getInternalType() == btCollisionObject::CO_RIGID_BODY )
		{
			const VuRigidBody *pRB = static_cast<const VuRigidBody *>(colObj);
			const btCollisionShape *colShape = colObj->getCollisionShape();
			const btTransform &transform = colObj->getWorldTransform();

			btVector3 center;
			float radius;
			colShape->getBoundingSphere(center, radius);
			center = transform*center;
			if ( camera.isSphereVisible(VuVector3(center.getX(), center.getY(), center.getZ()), radius) )
			{
				const VuColor &color = VuDynamics::IF()->getSurfaceColor(pRB->getSurfaceType());
				VuMatrix worldTransform = VuDynamicsUtil::toVuMatrix(colObj->getWorldTransform());
				drawCollisionObject(camera, worldTransform, colShape, color);
			}
		}
	}
}

//*****************************************************************************
void VuDynamicsDrawCollisionImpl::drawCollisionObject(const VuCamera &camera, const VuMatrix &worldTransform, const btCollisionShape *shape, const VuColor &color)
{
	if ( VuCollisionMeshAsset *pAsset = static_cast<VuCollisionMeshAsset *>(shape->getUserPointer()) )
	{
		VuMatrix transform = worldTransform;
		transform.scaleLocal(VuDynamicsUtil::toVuVector3(shape->getLocalScaling()));
		pAsset->drawWithColors(transform);
	}
	else if ( shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE )
	{
		VuMatrix transform = worldTransform;
		transform.scaleLocal(VuDynamicsUtil::toVuVector3(shape->getLocalScaling()));

		const btCompoundShape *compoundShape = static_cast<const btCompoundShape*>(shape);
		for ( int i = compoundShape->getNumChildShapes() - 1; i >= 0; i-- )
		{
			VuMatrix childTrans = VuDynamicsUtil::toVuMatrix(compoundShape->getChildTransform(i));
			const btCollisionShape *colShape = compoundShape->getChildShape(i);
			drawCollisionObject(camera, childTrans*transform, colShape, color);
		}
	}
	else
	{
		switch ( shape->getShapeType() )
		{
			case BOX_SHAPE_PROXYTYPE:
			{
				const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
				btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
				VuAabb aabb;
				aabb.mMin = VuVector3(-halfExtents.getX(), -halfExtents.getY(), -halfExtents.getZ());
				aabb.mMax = VuVector3( halfExtents.getX(),  halfExtents.getY(),  halfExtents.getZ());

				VuGfxUtil::IF()->drawAabbSolid(color, aabb, worldTransform, camera.getViewProjMatrix());

				break;
			}

			case SPHERE_SHAPE_PROXYTYPE:
			{
				const btSphereShape* sphereShape = static_cast<const btSphereShape*>(shape);
				btScalar radius = sphereShape->getMargin();//radius doesn't include the margin, so draw with margin

				VuGfxUtil::IF()->drawSphereSolid(color, radius, 8, 8, worldTransform, camera.getViewProjMatrix());

				break;
			}
			case MULTI_SPHERE_SHAPE_PROXYTYPE:
			{
				const btMultiSphereShape* multiSphereShape = static_cast<const btMultiSphereShape*>(shape);

				VuMatrix childMat;
				childMat.loadIdentity();

				for (int i = multiSphereShape->getSphereCount()-1; i>=0;i--)
				{
					childMat.setTrans(VuDynamicsUtil::toVuVector3(multiSphereShape->getSpherePosition(i)));
					VuGfxUtil::IF()->drawSphereSolid(color, multiSphereShape->getSphereRadius(i), 8, 8, worldTransform*childMat, camera.getViewProjMatrix());
				}

				break;
			}
			case CAPSULE_SHAPE_PROXYTYPE:
			{
				const btCapsuleShape* capsuleShape = static_cast<const btCapsuleShape*>(shape);

				btScalar radius = capsuleShape->getRadius();
				btScalar halfHeight = capsuleShape->getHalfHeight();

				// todo: handle up axis
				//int upAxis = capsuleShape->getUpAxis();

				VuMatrix shapeTrans = worldTransform;

				shapeTrans.rotateYLocal(VU_PIDIV2);
				VuGfxUtil::IF()->drawCapsuleSolid(color, 2.0f*halfHeight, radius, 8, shapeTrans, camera.getViewProjMatrix());

				break;
			}
			case CONE_SHAPE_PROXYTYPE:
			{
				const btConeShape* coneShape = static_cast<const btConeShape*>(shape);
                
				// todo: handle up axis
				//int upAxis = coneShape->getConeUpIndex();
                
				btScalar radius = coneShape->getRadius();//+coneShape->getMargin();
				btScalar height = coneShape->getHeight();//+coneShape->getMargin();

				// todo: handle up axis and possibly fix height
				//btVector3 start = worldTransform.getOrigin();

				VuGfxUtil::IF()->drawConeSolid(color, height, radius, 8, worldTransform, camera.getViewProjMatrix());

				break;
			}
			case CYLINDER_SHAPE_PROXYTYPE:
			{
				const btCylinderShape* cylinder = static_cast<const btCylinderShape*>(shape);
				int upAxis = cylinder->getUpAxis();
				btScalar radius = cylinder->getRadius();
				btScalar halfHeight = cylinder->getHalfExtentsWithMargin()[upAxis];

				// todo: handle up axis and possibly fix height
				//btVector3 start = worldTransform.getOrigin();

				VuGfxUtil::IF()->drawCylinderSolid(color, 2.0f*halfHeight, radius, 8, worldTransform, camera.getViewProjMatrix());

				break;
			}
			case STATIC_PLANE_PROXYTYPE:
			{
				const btStaticPlaneShape* staticPlaneShape = static_cast<const btStaticPlaneShape*>(shape);
				btScalar planeConst = staticPlaneShape->getPlaneConstant();
				const btVector3& planeNormal = staticPlaneShape->getPlaneNormal();
				btVector3 planeOrigin = planeNormal * planeConst;
				btVector3 vec0,vec1;
				btPlaneSpace1(planeNormal,vec0,vec1);
				btScalar vecLen = 100.f;

				VuVector3 v[4];
				v[0] = VuDynamicsUtil::toVuVector3(planeOrigin - vec0*vecLen);
				v[1] = VuDynamicsUtil::toVuVector3(planeOrigin + vec0*vecLen);
				v[2] = VuDynamicsUtil::toVuVector3(planeOrigin - vec1*vecLen);
				v[3] = VuDynamicsUtil::toVuVector3(planeOrigin + vec1*vecLen);

				VuGfxUtil::IF()->drawTriangleStrip(color, v, 4, camera.getViewProjMatrix());

				break;
			}
			case CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE:
			{
				btConvexTriangleMeshShape* convexMesh = (btConvexTriangleMeshShape*) shape;

				///@todo pass camera, for some culling? no -> we are not a graphics lib
				btVector3 aabbMax(btScalar(BT_LARGE_FLOAT),btScalar(BT_LARGE_FLOAT),btScalar(BT_LARGE_FLOAT));
				btVector3 aabbMin(btScalar(-BT_LARGE_FLOAT),btScalar(-BT_LARGE_FLOAT),btScalar(-BT_LARGE_FLOAT));

				convexMesh->getMeshInterface()->InternalProcessAllTriangles(&sDebugDrawcallback,aabbMin,aabbMax);
				sDebugDrawcallback.draw(worldTransform, color);

				break;
			}
			/*
		default:
			{

				if (shape->isConcave())
				{
					btConcaveShape* concaveMesh = (btConcaveShape*) shape;

					///@todo pass camera, for some culling? no -> we are not a graphics lib
					btVector3 aabbMax(btScalar(BT_LARGE_FLOAT),btScalar(BT_LARGE_FLOAT),btScalar(BT_LARGE_FLOAT));
					btVector3 aabbMin(btScalar(-BT_LARGE_FLOAT),btScalar(-BT_LARGE_FLOAT),btScalar(-BT_LARGE_FLOAT));

					DebugDrawcallback drawCallback(getDebugDrawer(),worldTransform,color);
					concaveMesh->processAllTriangles(&drawCallback,aabbMin,aabbMax);

				}

				if (shape->getShapeType() == CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE)
				{
					btConvexTriangleMeshShape* convexMesh = (btConvexTriangleMeshShape*) shape;
					//todo: pass camera for some culling			
					btVector3 aabbMax(btScalar(BT_LARGE_FLOAT),btScalar(BT_LARGE_FLOAT),btScalar(BT_LARGE_FLOAT));
					btVector3 aabbMin(btScalar(-BT_LARGE_FLOAT),btScalar(-BT_LARGE_FLOAT),btScalar(-BT_LARGE_FLOAT));
					//DebugDrawcallback drawCallback;
					DebugDrawcallback drawCallback(getDebugDrawer(),worldTransform,color);
					convexMesh->getMeshInterface()->InternalProcessAllTriangles(&drawCallback,aabbMin,aabbMax);
				}


				/// for polyhedral shapes
				if (shape->isPolyhedral())
				{
					btPolyhedralConvexShape* polyshape = (btPolyhedralConvexShape*) shape;

					int i;
					for (i=0;i<polyshape->getNumEdges();i++)
					{
						btVector3 a,b;
						polyshape->getEdge(i,a,b);
						btVector3 wa = worldTransform * a;
						btVector3 wb = worldTransform * b;
						getDebugDrawer()->drawLine(wa,wb,color);

					}


				}
			}
			*/
		}
	}
}

//*****************************************************************************
void VuDynamicsDrawCollisionImpl::DebugDrawcallback::draw(const VuMatrix &transform, const VuColor &color)
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->collisionShader()->setConstants(pData->mTransform, pData->mColor);
			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLELIST, pData->mVertCount/3, pData + 1);
		}

		VuMatrix	mTransform;
		VuColor		mColor;
		int			mVertCount;
	};

	if ( mVerts.size() )
	{
		int dataSize = mVerts.size()*sizeof(mVerts[0]);
		DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData) + dataSize));
		pData->mTransform = transform;
		pData->mColor = color;
		pData->mVertCount = mVerts.size();
		VU_MEMCPY(pData + 1, dataSize, &mVerts[0], dataSize);

		VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, VuGfxUtil::IF()->collisionShader()->getMaterial(), VUNULL, &DrawData::callback);

		mVerts.clear();
	}
}