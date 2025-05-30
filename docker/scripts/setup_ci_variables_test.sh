# sets up the CI_* variables we're using for local testing

CI_COMMIT_REF_PROTECTED=false
CI_COMMIT_REF_NAME= 
CI_COMMIT_BEFORE_SHA=0000000000000000000000000000000000000000 
CI_COMMIT_SHA=001f69b535aa16139b61ed0a3141169a06d19c20
CI_COMMIT_TAG=
CI_COMMIT_BRANCH= 
CI_DEFAULT_BRANCH=trunk
CI_MERGE_REQUEST_CHANGED_PAGE_PATHS=
CI_MERGE_REQUEST_ID=
CI_MERGE_REQUEST_SOURCE_BRANCH_NAME=
CI_MERGE_REQUEST_TARGET_BRANCH_NAME=

case $1 in
	raw)
	CI_COMMIT_REF_PROTECTED=
	CI_COMMIT_BEFORE_SHA=
	CI_COMMIT_SHA=
	CI_DEFAULT_BRANCH=
	;;
	trunk)
	CI_COMMIT_REF_PROTECTED=true
	CI_COMMIT_BEFORE_SHA=
	CI_COMMIT_SHA=
	CI_DEFAULT_BRANCH=trunk
	CI_COMMIT_REF_NAME=trunk
	CI_COMMIT_BRANCH=trunk
	;;
    ap)
	CI_COMMIT_REF_PROTECTED=true
	CI_COMMIT_REF_NAME=hack-0.2.8-sty+ct+ap
	CI_COMMIT_BRANCH=hack-0.2.8-sty+ct+ap
	;;
	hack)
	CI_COMMIT_REF_PROTECTED=true
	CI_COMMIT_BEFORE_SHA=
	CI_COMMIT_SHA=
	CI_DEFAULT_BRANCH=hack
	CI_COMMIT_REF_NAME=hack
	CI_COMMIT_BRANCH=hack
	;;
	wip)
	CI_COMMIT_REF_PROTECTED=true
	CI_COMMIT_BEFORE_SHA=
	CI_COMMIT_SHA=
	CI_DEFAULT_BRANCH=wip
	CI_COMMIT_REF_NAME=wip
	CI_COMMIT_BRANCH=wip
	;;
    tag)
	CI_COMMIT_REF_NAME=v0.2.7.3.4
	CI_COMMIT_TAG=v0.2.7.3.4
	;;
    alpha)
	CI_COMMIT_REF_PROTECTED=true
	CI_COMMIT_REF_NAME=legacy_0.3.1
	CI_COMMIT_BRANCH=legacy_0.3.1
	;;
    beta)
	CI_COMMIT_REF_PROTECTED=true
	CI_COMMIT_REF_NAME=beta_0.3.1
	CI_COMMIT_BRANCH=beta_0.3.1
	;;
    rc)
	CI_COMMIT_REF_PROTECTED=true
	CI_COMMIT_REF_NAME=release_0.3.1
	CI_COMMIT_BRANCH=release_0.3.1
	;;
    release)
	CI_COMMIT_REF_PROTECTED=true
	CI_COMMIT_REF_NAME=v0.2.7.3.4
	CI_COMMIT_TAG=v0.2.7.3.4
	;;
    ppa)
	# for post-release pushes of ppa builds to newer ubuntu versions
	CI_COMMIT_REF_PROTECTED=true
	CI_COMMIT_REF_NAME=`git -C ${git_repo} describe --tag` || return $? || exit $?
	CI_COMMIT_TAG=${CI_COMMIT_REF_NAME}
	;;
    merge)
	CI_COMMIT_REF_NAME=release__0.3.3 # CAN BE ANYTHING, merger chooses
	CI_MERGE_REQUEST_CHANGED_PAGE_PATHS=
	CI_MERGE_REQUEST_ID=
	CI_MERGE_REQUEST_SOURCE_BRANCH_NAME=
	CI_MERGE_REQUEST_TARGET_BRANCH_NAME=
	CI_MERGE_REQUEST_ID=60167617
	CI_MERGE_REQUEST_SOURCE_BRANCH_NAME=mr_test
	CI_MERGE_REQUEST_TARGET_BRANCH_NAME=trunk
	;;
	*)
	echo "NO VALID CONFIGURATION SELECTED, available: raw (no CI), hack, wip, ap (Alpha Project), alpha, beta, rc, release, tag (unprotected), ppa (for manually pushing ppa builds), merge"
	exit 1
	;;
esac

export CI_COMMIT_REF_PROTECTED
export CI_COMMIT_REF_NAME
export CI_COMMIT_BEFORE_SHA
export CI_COMMIT_SHA
export CI_COMMIT_TAG
export CI_COMMIT_BRANCH 
export CI_DEFAULT_BRANCH
export CI_MERGE_REQUEST_CHANGED_PAGE_PATHS
export CI_MERGE_REQUEST_ID
export CI_MERGE_REQUEST_SOURCE_BRANCH_NAME
export CI_MERGE_REQUEST_TARGET_BRANCH_NAME
