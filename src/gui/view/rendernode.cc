#include <view/rendernode.h>
#include <float.h>
using namespace Cairo;
namespace cdroid{

RenderNode::RenderNode(){
    mX = mY = mZ =.0f;
    mAlpha  = 1.f;
    mScaleX = 1.f;
    mScaleY = 1.f;
    mElevation =.0f;
    mTranslationX = .0f;
    mTranslationY = .0f;
    mTranslationZ = .0f;
    mPivotX = mPivotY = 0.5f;
    mRotation  = .0f;
    mRotationX = .0f;
    mRotationY = .0f;
    mMatrix = identity_matrix();
}

bool RenderNode::hasIdentityMatrix()const{
    const bool rc= (mX==.0f) && (mY==.0f) && (mZ==.0f) &&
       (mTranslationX==.0f) && (mTranslationY==.0f) &&
       (mScaleX ==1.f) && (mScaleY==1.f) && (mRotation==.0f);
    return rc;
}

static inline float sdot(float a,float b,float c,float d){
    return a * b + c * d;
}

void RenderNode::getMatrix(Matrix&outMatrix)const{
    outMatrix = identity_matrix();
    const float px = (mRight - mLeft)*mPivotX;
    const float py = (mBottom- mTop)*mPivotY;
    outMatrix.translate(px,py);
    outMatrix.scale(mScaleX,mScaleY);
    outMatrix.rotate(mRotation*M_PI/180.f);
    outMatrix.translate(mTranslationX - px,mTranslationY - py);
}

void RenderNode::getInverseMatrix(Matrix&outMatrix)const{
    getMatrix(outMatrix);
    outMatrix.invert();
}

void RenderNode::setAlpha(float alpha){
    mAlpha = alpha;
}

float RenderNode::getAlpha()const{
    return mAlpha;
}

void RenderNode::setElevation(float elevation){
    mElevation = elevation;
}

float RenderNode::getElevation()const{
    return mElevation;
}

void RenderNode::setTranslationX(float x){
    mTranslationX = x;
}

float RenderNode::getTranslationX()const{
    return mTranslationX;
}

void RenderNode::setTranslationY(float y){
    mTranslationY = y;
}

float RenderNode::getTranslationY()const{
    return mTranslationY;
}

void RenderNode::setTranslationZ(float z){
    mTranslationZ = z;
}

float RenderNode::getTranslationZ()const{
    return mTranslationZ;
}

void RenderNode::setRotation(float angle){
    mRotation = angle;
}

float RenderNode::getRotation()const{
    return mRotation;
}

void RenderNode::setRotationX(float angle){
    mRotationX = angle;
}

float RenderNode::getRotationX()const{
    return mRotationX;
}

void RenderNode::setRotationY(float angle){
    mRotationY = angle;
}

float RenderNode::getRotationY()const{
    return mRotationY;
}

void RenderNode::setScaleX(float scale){
    mScaleX = scale;
}

float RenderNode::getScaleX()const{
    return mScaleX;
}

void RenderNode::setScaleY(float scale){
    mScaleY = scale;
}

float RenderNode::getScaleY()const{
    return mScaleY;
}

void RenderNode::setPivotX(float px){
    mPivotX = px;
}

float RenderNode::getPivotX()const{
    return mPivotX;
}

void RenderNode::setPivotY(float py){
    mPivotY = py;
}

float RenderNode::getPivotY()const{
    return mPivotY;
}

bool RenderNode::isPivotExplicitlySet()const{
    return mPivotX!=std::numeric_limits<float>::min()&&
               mPivotY!=std::numeric_limits<float>::min();
}

void RenderNode::setLeft(float left){
    mLeft = left;
}

void RenderNode::setTop(float top){
    mTop = top;
}

void RenderNode::setRight(float right){
    mRight = right;
}

void RenderNode::setBottom(float bottom){
    mBottom = bottom;
}

void RenderNode::setLeftTopRightBottom(float left,float top,float right,float bottom){
    mLeft = left;
    mTop = top;
    mRight = right;
    mBottom= bottom;
}

}
