#include <drawables/animatedstatelistdrawable.h>
namespace cdroid{

AnimatedStateListDrawable::AnimatedStateListDrawable():StateListDrawable(){
}

AnimatedStateListDrawable::AnimatedStateListDrawable(std::shared_ptr<AnimatedStateListDrawable::AnimatedStateListState> state)
  :StateListDrawable(state){
    std::shared_ptr<AnimatedStateListState> newState = std::make_shared<AnimatedStateListState>(state.get(), this);
    setConstantState(newState);
    onStateChange(getState());
    jumpToCurrentState();
}

bool AnimatedStateListDrawable::setVisible(bool visible, bool restart){
    const bool changed = StateListDrawable::setVisible(visible, restart);

    if (mTransition && (changed || restart)) {
        if (visible) {
            mTransition->start();
        } else {
            // Ensure we're showing the correct state when visible.
            jumpToCurrentState();
        }
    }
    return changed;
}

void AnimatedStateListDrawable::addState(const std::vector<int>&stateSet,Drawable* drawable, int id){
    if (drawable == nullptr) {
        throw "Drawable must not be null";
    }
    mState->addStateSet(stateSet, drawable, id);
    onStateChange(getState());
}

void AnimatedStateListDrawable::addTransition(int fromId, int toId,AnimatedStateListDrawable::Transition* transition, bool reversible){
    if (transition == nullptr) {
        throw "Transition drawable must not be null";
    }

    //mState->addTransition(fromId, toId, transition, reversible);
}

bool AnimatedStateListDrawable::isStateful()const {
    return true;
}

bool AnimatedStateListDrawable::onStateChange(const std::vector<int>&stateSet){
    const int targetIndex = mState->indexOfKeyframe(stateSet);
    bool changed = targetIndex != getCurrentIndex()
            && (selectTransition(targetIndex) || selectDrawable(targetIndex));

    // We need to propagate the state change to the current drawable, but
    // we can't call StateListDrawable.onStateChange() without changing the
    // current drawable.
    Drawable* current = getCurrent();
    if (current != nullptr) {
        changed |= current->setState(stateSet);
    }

    return changed;
}

bool AnimatedStateListDrawable::selectTransition(int toIndex){
    int fromIndex;
    Transition* currentTransition = mTransition;
    if (currentTransition != nullptr) {
        if (toIndex == mTransitionToIndex) {
            // Already animating to that keyframe.
            return true;
        } else if (toIndex == mTransitionFromIndex && currentTransition->canReverse()) {
            // Reverse the current animation.
            currentTransition->reverse();
            mTransitionToIndex = mTransitionFromIndex;
            mTransitionFromIndex = toIndex;
            return true;
        }

        // Start the next transition from the end of the current one.
        fromIndex = mTransitionToIndex;

        // Changing animation, end the current animation.
        currentTransition->stop();
    } else {
        fromIndex = getCurrentIndex();
    }

    // Reset state.
    mTransition = nullptr;
    mTransitionFromIndex = -1;
    mTransitionToIndex = -1;

    const int fromId = mState->getKeyframeIdAt(fromIndex);
    const int toId = mState->getKeyframeIdAt(toIndex);
    if (toId == 0 || fromId == 0) {
        // Missing a keyframe ID.
        return false;
    }

    const int transitionIndex = mState->indexOfTransition(fromId, toId);
    if (transitionIndex < 0) {
        // Couldn't select a transition.
        return false;
    }

    bool hasReversibleFlag = mState->transitionHasReversibleFlag(fromId, toId);

    // This may fail if we're already on the transition, but that's okay!
    selectDrawable(transitionIndex);

    Transition* transition = nullptr;
    Drawable* d = getCurrent();
    /*if (dynamic_cast<AnimationDrawable*>(d)) {
        bool reversed = mState->isTransitionReversed(fromId, toId);
        transition = new AnimationDrawableTransition((AnimationDrawable*) d,reversed, hasReversibleFlag);
    } else if (dynamic_cast<AnimatedVectorDrawable*>(d)) {
        bool reversed = mState->isTransitionReversed(fromId, toId);
        transition = new AnimatedVectorDrawableTransition((AnimatedVectorDrawable*) d, reversed, hasReversibleFlag);
    } else if (dynamic_cast<Animatable*>(d)) {
        transition = new AnimatableTransition((Animatable*) d);
    } else*/ {
        // We don't know how to animate this transition.
        return false;
    }

    transition->start();
    mTransition = transition;
    mTransitionFromIndex = fromIndex;
    mTransitionToIndex = toIndex;
    return true;
}

void AnimatedStateListDrawable::jumpToCurrentState(){
    StateListDrawable::jumpToCurrentState();

    if (mTransition != nullptr) {
        mTransition->stop();
        mTransition = nullptr;

        selectDrawable(mTransitionToIndex);
        mTransitionToIndex = -1;
        mTransitionFromIndex = -1;
    }
}

Drawable* AnimatedStateListDrawable::mutate() {
    if (!mMutated && StateListDrawable::mutate() == this) {
        mState->mutate();
        mMutated = true;
    }
    return this;
}

void AnimatedStateListDrawable::clearMutated(){
    StateListDrawable::clearMutated();
    mMutated = false;
}

std::shared_ptr<DrawableContainer::DrawableContainerState> AnimatedStateListDrawable::cloneConstantState(){
    return std::make_shared<AnimatedStateListState>(mState.get(), this);
}

void AnimatedStateListDrawable::setConstantState(std::shared_ptr<DrawableContainerState> state){
    StateListDrawable::setConstantState(state);

    if (dynamic_cast<AnimatedStateListState*>(state.get())) {
        mState = std::dynamic_pointer_cast<AnimatedStateListState>(state);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

AnimatedStateListDrawable::AnimatedStateListState::AnimatedStateListState(const AnimatedStateListDrawable::AnimatedStateListState* orig,AnimatedStateListDrawable* owner)
  :StateListState(orig,owner){
}

void AnimatedStateListDrawable::AnimatedStateListState::mutate() {
    //mTransitions = mTransitions.clone();
    //mStateIds = mStateIds.clone();
}

int AnimatedStateListDrawable::AnimatedStateListState::addTransition(int fromId, int toId,Drawable* anim, bool reversible){
    const int pos = addChild(anim);
    const long keyFromTo = generateTransitionKey(fromId, toId);
    long reversibleBit = 0;
    if (reversible) {
        reversibleBit = REVERSIBLE_FLAG_BIT;
    }
    mTransitions.put(keyFromTo, pos | reversibleBit);//append

    if (reversible) {
        const long keyToFrom = generateTransitionKey(toId, fromId);
        mTransitions.put(keyToFrom, pos | REVERSED_BIT | reversibleBit);//append
    }
    return pos;
}

int AnimatedStateListDrawable::AnimatedStateListState::addStateSet(std::vector<int> stateSet,Drawable*drawable, int id){
    const int index = StateListDrawable::StateListState::addStateSet(stateSet, drawable);
    mStateIds.put(index, id);
    return index;
}

int AnimatedStateListDrawable::AnimatedStateListState::indexOfKeyframe(const std::vector<int>&stateSet) {
    const int index = StateListDrawable::StateListState::indexOfStateSet(stateSet);
    if (index >= 0) {
        return index;
    }

    return StateListDrawable::StateListState::indexOfStateSet(StateSet::WILD_CARD);
}

int AnimatedStateListDrawable::AnimatedStateListState::getKeyframeIdAt(int index) {
    return index < 0 ? 0 : mStateIds.get(index, 0);
}

int AnimatedStateListDrawable::AnimatedStateListState::indexOfTransition(int fromId, int toId) {
    const long keyFromTo = generateTransitionKey(fromId, toId);
    return (int) mTransitions.get(keyFromTo, -1);
}

bool AnimatedStateListDrawable::AnimatedStateListState::isTransitionReversed(int fromId, int toId) {
    const long keyFromTo = generateTransitionKey(fromId, toId);
    return (mTransitions.get(keyFromTo, -1) & REVERSED_BIT) != 0;
}

bool AnimatedStateListDrawable::AnimatedStateListState::transitionHasReversibleFlag(int fromId, int toId) {
    const long keyFromTo = generateTransitionKey(fromId, toId);
    return (mTransitions.get(keyFromTo, -1) & REVERSIBLE_FLAG_BIT) != 0;
}

//bool canApplyTheme() {return mAnimThemeAttrs != null || super.canApplyTheme();}
Drawable* AnimatedStateListDrawable::AnimatedStateListState::newDrawable() {
    return new AnimatedStateListDrawable(std::dynamic_pointer_cast<AnimatedStateListState>(shared_from_this()));
}

long AnimatedStateListDrawable::AnimatedStateListState::generateTransitionKey(int fromId, int toId) {
    return (long) fromId << 32 | toId;
}

//---------------------------------------------------------------------------------------------------------------
AnimatedStateListDrawable::FrameInterpolator::FrameInterpolator(AnimationDrawable* d, bool reversed){
     updateFrames(d, reversed);
}

int  AnimatedStateListDrawable::FrameInterpolator::updateFrames(AnimationDrawable* d, bool reversed){
    int N = d->getNumberOfFrames();
    mFrames = N;

    if (mFrameTimes.size() < N) {
        mFrameTimes.resize(N);
    }

    std::vector<int>&frameTimes = mFrameTimes;
    int totalDuration = 0;
    for (int i = 0; i < N; i++) {
        const int duration = d->getDuration(reversed ? N - i - 1 : i);
        frameTimes[i] = duration;
        totalDuration += duration;
    }

    mTotalDuration = totalDuration;
    return totalDuration;
}

int  AnimatedStateListDrawable::FrameInterpolator::getTotalDuration(){
    return mTotalDuration;
}

float AnimatedStateListDrawable::FrameInterpolator::getInterpolation(float input){
    const int elapsed = (int) (input * mTotalDuration + 0.5f);
    const int N = mFrames;
    const std::vector<int>& frameTimes = mFrameTimes;

    // Find the current frame and remaining time within that frame.
    int remaining = elapsed;
    int i = 0;
    while (i < N && remaining >= frameTimes[i]) {
        remaining -= frameTimes[i];
        i++;
    }

    // Remaining time is relative of total duration.
    const float frameElapsed= (i<N)?(remaining / (float) mTotalDuration):.0f;
    return i / (float) N + frameElapsed;
}

//----------------------------------------------------------------------------------------------------------
}//endof namespace