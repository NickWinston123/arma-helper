.PHONY: tarball clean distclean source free closed full all

# We want everything deleted if something bad happens
.DELETE_ON_ERROR:

default:
	echo "Nothing generic to do."

# individual targets for split jobs
CI_prepare: gits_artifacts.tag source make_ci.sh rootcontext.64
CI_linux_server_32: appimage_server_32.tag apptar_server_32.tag
CI_linux_client_32: appimage_client_32.tag apptar_client_32.tag
CI_linux_server_64: appimage_server_64.tag apptar_server_64.tag
CI_linux_client_64: appimage_client_64.tag apptar_client_64.tag
CI_windows: winzip.tag windows.tag
CI_debtest: debtest.tag
CI_linux_steam: steamdirs/steam_linux
CI_windows_steam: steamdirs/steam_windows

# full open source builds
free.tag: appimage.tag apptar.tag sourcetar.tag
	touch $@
free: free.tag

# builds for non-open source systems/environments
closed_regular.tag: windows.tag winzip.tag
	touch $@
closed_regular: closed.tag
closed_steam.tag: steam_windows.tag steam_linux.tag
	touch $@
closed_steam: closed_steam.tag
closed.tag: closed_steam.tag closed_regular.tag
	touch $@
closed: closed.tag

# closed comes first because windows builds take the longest and are not internally parallelized
full.tag: result.winsource_32 result.winsource_steam closed.tag free.tag
	touch $@
full: full.tag

# what to build from GitLab CI, divided into stages
CI_base.tag: gits.tag source.tag result.build_server_32 result.build_client_64 context.64
	touch $@

# minimal expected build to be packable
CI_pack.tag: gits_artifacts.tag source.tag CI_base.tag free.tag upload/PATCHNOTES.md upload/CHANGELOG.md upload/RELEASENOTES.md
	touch $@

# all windows builds
CI_windows.tag: CI_base.tag steam_windows.tag closed_regular.tag
	touch $@

# all free linux builds (so, no steam)
CI_free.tag: CI_base.tag free.tag reformat.tag
	touch $@

# (OBSOLETE) targets for CI merge tests, a bit leaner than a full build
CI_merge.tag: windows.tag debtest.tag
	touch $@
CI_merge: CI_merge.tag
CI_merge_nodeb.tag: windows.tag CI_base.tag
	touch $@
CI_merge_nodeb: CI_merge_nodeb.tag

# all builds
#CI.tag: CI_pack.tag apptar_server_64.tag
CI.tag: CI_windows.tag CI_free.tag full.tag CI_pack.tag
	touch $@

CI_base: CI_base.tag
CI: CI.tag

# all tests
test: debtest.tag appimagetests.tag CI.tag deploy_base.tag

all:	default

dockertest.tag:
	cp ${srcdir}/fakerelease_proto.sh .
	@echo "Testing whether 'docker' works..."
	if which docker; then docker help > /dev/null; fi
	echo > $@

tarwrapper.sh: ${top_srcdir}/docker/scripts/tarwrapper.sh
	ln -s $< $@

#***************************************************************

# variables

# package metadata
PACKAGE_VERSION:=$(shell grep "^PACKAGE_VERSION=" context/version.sh | sed -e "s,.*=,,")
PACKAGE_NAME:=$(shell grep "^PACKAGE_NAME=" context/version.sh | sed -e "s,.*=,,")
PACKAGE_TITLE:=$(shell grep "^PACKAGE_TITLE=" context/version.sh | sed -e "s,.*=,,")
PACKAGE_NAME_SUFFIX:=$(shell echo $(PACKAGE_NAME) | sed -e "s,.*-,-,")
PACKAGE_TITLE_SUFFIX:=$(shell echo $(PACKAGE_TITLE) | sed -e "s,.* , ,")
STEAM_PACKAGE_NAME=retrocycles
STEAM_PACKAGE_TITLE=Retrocycles

# configurations for lean builds not supposed to be installed
CONFIGURE_ARGUMENTS:=--disable-uninstall --disable-restoreold --enable-etc --enable-automakedefaults --disable-useradd --disable-sysinstall --disable-initscripts
CONFIGURE_ARGUMENTS_SERVER:=--disable-glout --enable-armathentication

# other directories
SCRIPTDIR:=${srcdir}/../scripts
GITS:=$(SCRIPTDIR)/.cache/gits

# dump CI data and variables determined from it
CI_INFO: ${srcdir}/../scripts/brand.sh
	. ${srcdir}/../scripts/brand.sh @top_srcdir@ && \
	echo SERIES=$${SERIES} > $@ && \
	echo PROGRAM_NAME=$${PROGRAM_NAME} >> $@ && \
	echo PROGRAM_TITLE=$${PROGRAM_TITLE} >> $@ && \
	echo CI_COMMIT_REF_PROTECTED=$${CI_COMMIT_REF_PROTECTED} >> $@ && \
	echo CI_COMMIT_SHA=$${CI_COMMIT_SHA} >> $@ && \
	echo CI_RELEASE_YEAR=$${CI_RELEASE_YEAR} >> $@ && \
	echo CI_COMMIT_REF_NAME=$${CI_COMMIT_REF_NAME} >> $@

.PRECIOUS: CI_INFO

#***************************************************************

upload/.tag:
	mkdir -p upload
	echo > $@
proto.tag:
	mkdir -p proto
	echo > $@

make_ci.sh: ${srcdir}/make_ci.sh.in
	cp $< $@

context/version.sh: ${srcdir}/context/version.sh.in
	cd ../../; ./config.status docker/build/context/version.sh

../deploy/targets.sh: ${srcdir}/../deploy/targets.sh.in
	cd ../../; ./config.status docker/deploy/targets.sh

#***************************************************************

# create source tarball
TARBALL:=../../armagetronad-@version@.tar.gz
$(TARBALL):
	$(MAKE) -C ../../ dist GZIP_ENV=""
tarball: $(TARBALL)

# unpack source tarball
source.tag: $(TARBALL) dockertest.tag
	rm -rf source armagetronad-@version@
	tar -xzf $(TARBALL)
	mv armagetronad-@version@ source
	echo > $@
source: source.tag

# rebranded tarball for upload
upload/$(PACKAGE_NAME)-@version@.tbz: $(TARBALL) upload/.tag tarwrapper.sh
	rm -rf $@ repack
	mkdir -p repack_1 repack_2
	cd repack_1; tar -xzf ../$(TARBALL)
	mv repack_1/armagetronad-@version@ repack_2/$(PACKAGE_NAME)-@version@
	cd repack_2; ../tarwrapper.sh -cjf repacked.tbz $(PACKAGE_NAME)-@version@
	mv repack_2/repacked.tbz $@
	rm -rf repack_1 repack_2

sourcetar.tag: upload/$(PACKAGE_NAME)-@version@.tbz
	touch $@

# foreign gits (in the cache)
gits.tag: $(SCRIPTDIR)/fix_gits.sh
	$(SCRIPTDIR)/fix_gits.sh
	echo > $@

# bits of gits we need in artifacts
gits_artifacts.tag: gits.tag
	rm -rf gits
	mkdir -p gits
	mkdir -p gits/steam
	cp -a ${GITS}/steam-art/good/title.png gits/steam
	echo > $@

upload/PATCHNOTES.md: sourcetar.tag upload/.tag
	${top_srcdir}/batch/make/patchnotes.py ${top_srcdir} ${top_srcdir}/CHANGELOG_FROZEN.md armagetronad armagetronad -p > $@

upload/CHANGELOG.md: sourcetar.tag upload/.tag
	${top_srcdir}/batch/make/patchnotes.py ${top_srcdir} ${top_srcdir}/CHANGELOG_FROZEN.md armagetronad armagetronad > $@

upload/RELEASENOTES.md: ${src_dir}../deploy/RELEASENOTES.md upload/.tag
	cp $< $@

#***************************************************************

# generic context with source
# technically everything here depends on the Makefile, but enforcing rebuilds on all
# Makefile edits is too annoying during develop/test cycles.
rootcontext.64: source.tag ${srcdir}/*.sh ${srcdir}/context/build.sh #Makefile
	${srcdir}/prepare_context.sh $@.dir source ${srcdir}/context/build.sh
	echo "armabuild_64" > $@.dir/image
	echo > $@

context.64: rootcontext.64
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*'
	echo > $@

#***************************************************************

# switch container to use to 32 bit
context.32: context.64
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*'
	rm $@.dir/image
	echo "armabuild_32" > $@.dir/image
	echo > $@

# switch container to use to steam SDK, rebrand
context.steam: context.64 gits_artifacts.tag
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*'
	rm $@.dir/image
	echo "armasteam_64" > $@.dir/image
#	rebrand
	rm -f $@.dir/version.sh # required because the source and dest are hard links to the same file originally
	sed < $<.dir/version.sh > $@.dir/version.sh -e s/PACKAGE_NAME=armagetronad/PACKAGE_NAME=$(STEAM_PACKAGE_TITLE)/ -e s/PACKAGE_TITLE=\"Armagetron\ Advanced/PACKAGE_TITLE=\"$(STEAM_PACKAGE_TITLE)/ -e s/PACKAGE_TITLE=\"Armagetron/PACKAGE_TITLE=\"$(STEAM_PACKAGE_TITLE)/
# 	no remotely adult stuff
	rm -f $@.dir/source/src/engine/ePlayer.cpp
	sed < $<.dir/source/src/engine/ePlayer.cpp > $@.dir/source/src/engine/ePlayer.cpp -e "s/pr0n/cat videos/"
#   install alternative title, adapt sources
	cp gits/steam/title.png $@.dir/source/textures/
	for f in source/src/tron/gLogo.cpp source/Makefile.am; do \
		rm -f $@.dir/$$f; \
		sed < $<.dir/$$f > $@.dir/$$f -e "s,textures/title.jpg,textures/title.png,"; \
	done
	cd $@.dir/source; automake
#   done
	echo > $@

# add debian/ubuntu build info for release %. Requires GPG key from ~/.gnupg
context.debsrc_%: rootcontext.64 ${srcdir}/rebrand_debian_core.sh ${srcdir}/debian_source.sh gits_artifacts.tag $(TARBALL)
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*' $(TARBALL) ${srcdir}/rebrand_debian_core.sh ${srcdir}/debian_source.sh ${srcdir}/context/debian
	if test -r  ../deploy/secrets/sec.gpg; then cp -lrf ../deploy/secrets/*.gpg $@.dir/; fi
	rm -f $@.dir/image
	echo "armadeb_64" > $@.dir/image
	echo > $@
result.debsrc_%: context.debsrc_%
	+$(srcdir)/context_to_result.sh $@.dir ./debian_source.sh $*
	echo > $@

# test build a debian package (with a really old ubuntu codename)
context.debsrc_test: context.debsrc_dapper
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*'
#	extract fake secrets
	base64 -d < ${srcdir}/../deploy/anonymous_secrets.b64 | tar -xjf - --one-top-level=$@.dir
#   reverse to pack: tar -cjf - secrets | base64 > anonymous_secrets.b64 
	mv $@.dir/secrets/* $@.dir
	rmdir $@.dir/secrets/
	echo > $@
context.debtest: result.debsrc_test
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*'
	echo "cd source && debuild -i -us -uc -b && rm -rf *" > $@.dir/build_deb.sh
	chmod +x $@.dir/build_deb.sh
	echo > $@

result.debtest: context.debtest
	+$(srcdir)/context_to_result.sh $@.dir bash ./build_deb.sh
	echo > $@

debtest.tag: result.debtest
	echo > $@

# switch container to use alpine
context.test: rootcontext.64
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*'
	rm $@.dir/image
	echo "armalpine_32" > $@.dir/image
	echo > $@

#***************************************************************

# macOS builds are done without docker
MACOS_BUILD_BUNDLE="src/macosx/build_bundle.sh"
MACOS_CONFIGURE_BUNDLE="src/macosx/configure_for_bundle.sh"

MACZIP_CLIENT:=$(PACKAGE_NAME)-client-$(PACKAGE_VERSION).macOS.zip
MACZIP_SERVER:=$(PACKAGE_NAME)-server-$(PACKAGE_VERSION).macOS.zip

macOS/steam_build.tag: context.steam #macOS/client_build.tag #Makefile
	mkdir -p macOS/steam
	. context.steam.dir/version.sh && cd macOS/steam && ../../$<.dir/source/$(MACOS_CONFIGURE_BUNDLE) progname="$${PACKAGE_NAME}-${PACKAGE_VERSION}" progtitle="$${PACKAGE_TITLE}" && ${MAKE}
	echo > $@

macOS/server_build.tag: context.64 #smacOS/client_build.tag #Makefile
	mkdir -p macOS/server
	cd macOS/server && ../../$<.dir/source/$(MACOS_CONFIGURE_BUNDLE) --disable-glout && ${MAKE}
	echo > $@

macOS/client_build.tag: context.64 #Makefile
	mkdir -p macOS/client
	cd macOS/client && ../../$<.dir/source/$(MACOS_CONFIGURE_BUNDLE) && ${MAKE}
	echo > $@


macOS/%_sign.tag: macOS/%_build.tag #Makefile
	@echo /////////////////////////////////////////
	@echo // Bundle, sign and notarize macOS $*
	@echo /////////////////////////////////////////
	rm -rf macOS/$*/*.app macOS/$*/*.zip macOS/$*/*.dmg 
	cd macOS/$* && SKIP_PART_2=yes bash ${MACOS_BUILD_BUNDLE}
	echo > $@

macOS/%.tag: macOS/%_sign.tag upload/.tag #Makefile
	@echo /////////////////////////////////
	@echo // Staple and pack macOS $*
	@echo /////////////////////////////////
	cd macOS/$* && SKIP_PART_1=yes bash ${MACOS_BUILD_BUNDLE}
	cp macOS/$*/*.dmg macOS/$*/*.macOS.zip upload/
	echo > $@

#macOS/server_sign.tag: macOS/steam_sign.tag
#macOS/client_sign.tag: macOS/server_sign.tag
macOS/server.tag: macOS/steam.tag
macOS/client.tag: macOS/server.tag

.PRECIOUS: macOS/server_build.tag macOS/client_build.tag macOS/steam_build.tag
.PRECIOUS: macOS/server_sign.tag macOS/client_sign.tag macOS/steam_sign.tag
.PRECIOUS: macOS/server.tag macOS/client.tag macOS/steam.tag

macOS_build: macOS/client_build.tag macOS/server_build.tag macOS/steam_build.tag
macOS: macOS/client.tag macOS/server.tag macOS/steam.tag
upload/${MACZIP_CLIENT}: macOS/client.tag
upload/${MACZIP_SERVER}: macOS/server.tag

#***************************************************************

context.build_server_%: context.%
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*'
	echo "server" > $@.dir/serverclient
	echo > $@

context.build_client_%: context.%
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*'
	echo "client" > $@.dir/serverclient
	echo > $@

.PRECIOUS: context.build_server_32 context.build_client_32  context.build_server_64 context.build_client_64

#***************************************************************

# server build for further processing
result.build_server_%: context.build_server_%
	+$(srcdir)/context_to_result.sh $@.dir ./build.sh $(CONFIGURE_ARGUMENTS) $(CONFIGURE_ARGUMENTS_SERVER)
	echo > $@

# client build for further processing
result.build_client_%: context.build_client_%
	+$(srcdir)/context_to_result.sh $@.dir ./build.sh $(CONFIGURE_ARGUMENTS)
	echo > $@

.PRECIOUS: result.build_server_32 result.build_client_32  result.build_server_64 result.build_client_64

#***************************************************************

# portable application directories
context.appdir_%: result.build_% ${srcdir}/context/portable/* ${srcdir}/context/appdir.sh ${srcdir}/context/steam.sh
	${srcdir}/prepare_context.sh $@.dir $<.dir/* ${srcdir}/context/portable ${srcdir}/context/appdir.sh ${srcdir}/context/steam.sh
	echo > $@

result.appdir_%: context.appdir_%
	+$(srcdir)/context_to_result.sh $@.dir ./appdir.sh
	echo > $@

# use different script for steam builds, rearranging some things
result.appdir_%_steam: context.appdir_%_steam
	+$(srcdir)/context_to_result.sh $@.dir ./steam.sh
	echo > $@

# tarball of appdir
APPTAR_SERVER_32=$(PACKAGE_NAME)-server_32-$(PACKAGE_VERSION).tbz
APPTAR_SERVER_64=$(PACKAGE_NAME)-server_64-$(PACKAGE_VERSION).tbz
APPTAR_CLIENT_32=$(PACKAGE_NAME)-client_32-$(PACKAGE_VERSION).tbz
APPTAR_CLIENT_64=$(PACKAGE_NAME)-client_64-$(PACKAGE_VERSION).tbz
apptar_server_32.tag: upload/$(APPTAR_SERVER_32)
	echo > $@
apptar_server_64.tag: upload/$(APPTAR_SERVER_64)
	echo > $@
apptar_client_32.tag: upload/$(APPTAR_CLIENT_32)
	echo > $@
apptar_client_64.tag: upload/$(APPTAR_CLIENT_64)
	echo > $@

upload/$(APPTAR_SERVER_32): result.appdir_server_32 upload/.tag tarwrapper.sh
	cd $<.dir/appdir; ../../tarwrapper.sh -cjf ../../$@ .
upload/$(APPTAR_CLIENT_32): result.appdir_client_32 upload/.tag tarwrapper.sh
	cd $<.dir/appdir; ../../tarwrapper.sh -cjf ../../$@ .
upload/$(APPTAR_SERVER_64): result.appdir_server_64 upload/.tag tarwrapper.sh
	cd $<.dir/appdir; ../../tarwrapper.sh -cjf ../../$@ .
upload/$(APPTAR_CLIENT_64): result.appdir_client_64 upload/.tag tarwrapper.sh
	cd $<.dir/appdir; ../../tarwrapper.sh -cjf ../../$@ .

apptar.tag: apptar_server_32.tag apptar_client_32.tag apptar_server_64.tag apptar_client_64.tag upload/$(APPTAR_SERVER_32) upload/$(APPTAR_SERVER_64) upload/$(APPTAR_CLIENT_32) upload/$(APPTAR_CLIENT_64)
	echo > $@
apptar: apptar.tag

.PHONY: appdir apptar

.PRECIOUS: context.apptar_server_32 context.apptar_client_32  context.apptar_server_64 context.apptar_client_64

.PRECIOUS: context.appdir_server_32 context.appdir_client_32  context.appdir_server_64 context.appdir_client_64

#***************************************************************

# Docker image building
build_docker.tag: rootcontext.64 ${srcdir}/../deploy/deploy_docker.sh CI_INFO
	DOCKER_NODEPLOY="nodeploy" ${srcdir}/../deploy/deploy_docker.sh $<.dir/source
	echo > $@

deploy_docker.tag: rootcontext.64 ${srcdir}/../deploy/deploy_docker.sh CI_INFO
	${srcdir}/../deploy/deploy_docker.sh $<.dir/source
	echo > $@

#***************************************************************

# AppImages with optional signing
context.appimage_%: upload/$(PACKAGE_NAME)-%-$(PACKAGE_VERSION).tbz context/version.sh ${srcdir}/context/appimage.sh gits.tag
	${srcdir}/prepare_context.sh $@.dir ${srcdir}/context/appimage.sh $(GITS)/pkg2appimage
	mkdir -p $@.dir/appdir/ && tar -jxf $< -C $@.dir/appdir/
	echo $* | sed -e 's,.*_,armabuild_,' > $@.dir/image
	if test -r  ../deploy/secrets/sec.gpg; then cp -lrf ../deploy/secrets/*.gpg $@.dir/; fi
	echo > $@

APPIMAGE_BASENAME:=$(shell echo $(PACKAGE_TITLE) | sed -e "s, ,,g")
APPIMAGE_CLIENT_64:=$(APPIMAGE_BASENAME).AppImage
APPIMAGE_SERVER_64:=$(APPIMAGE_BASENAME)Dedicated.AppImage
APPIMAGE_CLIENT_32:=$(APPIMAGE_BASENAME)-32bit.AppImage
APPIMAGE_SERVER_32:=$(APPIMAGE_BASENAME)Dedicated-32bit.AppImage

result.appimage_client_64: context.appimage_client_64
	+$(srcdir)/context_to_result.sh $@.dir "./appimage.sh ${APPIMAGE_CLIENT_64}"
	echo > $@
result.appimage_server_64: context.appimage_server_64
	+$(srcdir)/context_to_result.sh $@.dir "./appimage.sh ${APPIMAGE_SERVER_64}"
	echo > $@
result.appimage_client_32: context.appimage_client_32
	+$(srcdir)/context_to_result.sh $@.dir "./appimage.sh ${APPIMAGE_CLIENT_32}"
	echo > $@
result.appimage_server_32: context.appimage_server_32
	+$(srcdir)/context_to_result.sh $@.dir "./appimage.sh ${APPIMAGE_SERVER_32}"
	echo > $@

upload/$(APPIMAGE_CLIENT_64): result.appimage_client_64 upload/.tag
	cp $<.dir/`basename $@` $@
	cp $<.dir/`basename $@`.zsync $@.zsync
upload/$(APPIMAGE_SERVER_64): result.appimage_server_64 upload/.tag
	cp $<.dir/`basename $@` $@
	cp $<.dir/`basename $@`.zsync $@.zsync
upload/$(APPIMAGE_CLIENT_32): result.appimage_client_32 upload/.tag
	cp $<.dir/`basename $@` $@
	cp $<.dir/`basename $@`.zsync $@.zsync
upload/$(APPIMAGE_SERVER_32): result.appimage_server_32 upload/.tag
	cp $<.dir/`basename $@` $@
	cp $<.dir/`basename $@`.zsync $@.zsync

appimage_client_64.tag: upload/$(APPIMAGE_CLIENT_64)
	echo > $@
appimage_server_64.tag: upload/$(APPIMAGE_SERVER_64)
	echo > $@
appimage_client_32.tag: upload/$(APPIMAGE_CLIENT_32)
	echo > $@
appimage_server_32.tag: upload/$(APPIMAGE_SERVER_32)
	echo > $@

appimage.tag: appimage_client_64.tag appimage_server_64.tag appimage_client_32.tag appimage_server_32.tag
	echo > $@

.PRECIOUS: context.appimage_server_32 context.appimage_client_32 context.appimage_server_64 context.appimage_client_64

#***************************************************************

# appimage tests; test scripts are in contest/tests/Dockerfile.*

test.appimage_server_64_ubuntu_latest: upload/$(APPIMAGE_SERVER_64)
test.appimage_server_64_ubuntu_oldest: upload/$(APPIMAGE_SERVER_64)
test.appimage_client_64_ubuntu_latest: upload/$(APPIMAGE_CLIENT_64)
test.appimage_client_64_ubuntu_oldest: upload/$(APPIMAGE_CLIENT_64)

test.appimage_server_64_ubuntu_rolling: upload/$(APPIMAGE_SERVER_64)
test.appimage_client_64_ubuntu_rolling: upload/$(APPIMAGE_CLIENT_64)

test.appimage_client_64_ubuntu_latest_skewed: upload/$(APPIMAGE_CLIENT_64)
test.appimage_client_64_ubuntu_oldest_skewed: upload/$(APPIMAGE_CLIENT_64)

test.appimage_server_64_fedora_latest: upload/$(APPIMAGE_SERVER_64)
test.appimage_server_64_fedora_oldest: upload/$(APPIMAGE_SERVER_64)
test.appimage_client_64_fedora_latest: upload/$(APPIMAGE_CLIENT_64)
test.appimage_client_64_fedora_oldest: upload/$(APPIMAGE_CLIENT_64)

test.appimage_server_64_centos_latest: upload/$(APPIMAGE_SERVER_64)
test.appimage_server_64_centos_oldest: upload/$(APPIMAGE_SERVER_64)
test.appimage_client_64_centos_latest: upload/$(APPIMAGE_CLIENT_64)
test.appimage_client_64_centos_oldest: upload/$(APPIMAGE_CLIENT_64)

test.appimage_server_32_debian_latest: upload/$(APPIMAGE_SERVER_32)
test.appimage_server_32_debian_oldest: upload/$(APPIMAGE_SERVER_32)
test.appimage_client_32_debian_latest: upload/$(APPIMAGE_CLIENT_32)
test.appimage_client_32_debian_oldest: upload/$(APPIMAGE_CLIENT_32)

test.%: context/tests/Dockerfile.%
	rm -rf $@.dir
	mkdir -p $@.dir
	cp $^ $@.dir
	cd $@.dir; docker rmi $*:last || true
	cd $@.dir; docker tag $*:latest $*:last || true
	cd $@.dir; docker build . -f Dockerfile.* --pull -t $*:latest
	cd $@.dir; docker rmi $*:last || true
	echo > $@

appimagetests.tag: \
	test.appimage_server_64_ubuntu_latest \
	test.appimage_server_64_ubuntu_oldest \
	test.appimage_client_64_ubuntu_latest \
	test.appimage_client_64_ubuntu_oldest \
	test.appimage_client_64_ubuntu_latest_skewed \
	test.appimage_client_64_ubuntu_oldest_skewed \
	test.appimage_server_64_ubuntu_rolling \
	test.appimage_client_64_ubuntu_rolling \
	test.appimage_server_64_fedora_latest \
	test.appimage_server_64_fedora_oldest \
	test.appimage_client_64_fedora_latest \
	test.appimage_client_64_fedora_oldest \
	test.appimage_server_64_centos_latest \
	test.appimage_server_64_centos_oldest \
	test.appimage_client_64_centos_latest \
	test.appimage_client_64_centos_oldest \
	test.appimage_server_32_debian_latest \
	test.appimage_server_32_debian_oldest \
	test.appimage_client_32_debian_latest \
	test.appimage_client_32_debian_oldest \
	upload/.tag
	echo > $@

appimagetests: appimagetests.tag

appimagetests_ci.tag: \
	test.appimage_client_64_ubuntu_latest \
	test.appimage_client_64_ubuntu_oldest_skewed \
	test.appimage_client_64_fedora_latest \
	test.appimage_server_64_centos_oldest \
	test.appimage_client_64_centos_oldest \
	test.appimage_server_32_debian_oldest \
	test.appimage_client_32_debian_oldest \
	upload/.tag
	echo > $@

appimagetests_ci: appimagetests_ci.tag

appimagetests_extra.tag: \
	test.appimage_server_64_ubuntu_rolling \
	test.appimage_client_64_ubuntu_rolling \
	upload/.tag
	echo > $@

appimagetests_extra: appimagetests_extra.tag

#***************************************************************

# windows source
context.winsource_%: result.build_server_% ${srcdir}/context/winsource.sh gits.tag
	${srcdir}/prepare_context.sh $@.dir $<.dir/* ${srcdir}/context/winsource.sh $(GITS)/codeblocks
	echo > $@
result.winsource_%: context.winsource_%
	+$(srcdir)/context_to_result.sh $@.dir ./winsource.sh
	echo > $@

# windows build
context.winbuild_%: result.winsource_% ${srcdir}/context/winbuild.sh gits.tag
	${srcdir}/prepare_context.sh $@.dir $<.dir/* ${srcdir}/context/winbuild.sh $(GITS)/winlibs
	rm $@.dir/image
	echo "armawineblocks" > $@.dir/image
	echo > $@
result.winbuild_%: context.winbuild_%
	+$(srcdir)/context_to_result.sh $@.dir ./winbuild.sh
	echo > $@

# windows installers
context.wininst_%: result.winbuild_% ${srcdir}/context/wininst.sh
	${srcdir}/prepare_context.sh $@.dir $<.dir/* ${srcdir}/context/wininst.sh
	rm $@.dir/image
	echo "armawineblocks" > $@.dir/image
	echo > $@
result.wininst_%: context.wininst_%
	+$(srcdir)/context_to_result.sh $@.dir ./wininst.sh
	echo > $@

# windows source zip
WINZIP_SOURCE_NAME:=$(PACKAGE_NAME)-source-$(PACKAGE_VERSION)
WINZIP_SOURCE_DIR:=proto/$(WINZIP_SOURCE_NAME)
WINZIP_SOURCE:=upload/$(WINZIP_SOURCE_NAME).zip
$(WINZIP_SOURCE_DIR): result.winsource_32 proto.tag Makefile
	rm -rf $@.dir
	cp -al $<.dir/winsource $@.dir
	echo > $@
$(WINZIP_SOURCE): $(WINZIP_SOURCE_DIR) upload/.tag
	cd $(WINZIP_SOURCE_DIR).dir; zip -9 -r ../../$(WINZIP_SOURCE) *
winzip_source.tag: $(WINZIP_SOURCE)
	echo > $@

# windows client zip
WINZIP_CLIENT_NAME:=$(PACKAGE_NAME)-client-$(PACKAGE_VERSION)
WINZIP_CLIENT_DIR:=proto/$(WINZIP_CLIENT_NAME)
WINZIP_CLIENT:=upload/$(WINZIP_CLIENT_NAME).win32.zip
$(WINZIP_CLIENT_DIR): result.winbuild_32 proto.tag Makefile
	rm -rf $@.dir
	cp -al $<.dir/dist $@.dir
	rm $@.dir/$(PACKAGE_NAME)_dedicated.exe $@.dir/*.nsi $@.dir/banner* $@.dir/INSTALL.txt
	echo > $@
$(WINZIP_CLIENT): $(WINZIP_CLIENT_DIR) upload/.tag
	cd $(WINZIP_CLIENT_DIR).dir; zip -9 -r ../../$(WINZIP_CLIENT) *
winzip_client.tag: $(WINZIP_CLIENT)
	echo > $@

# windows server zip
WINZIP_SERVER_NAME:=$(PACKAGE_NAME)-server-$(PACKAGE_VERSION)
WINZIP_SERVER_DIR:=proto/$(WINZIP_SERVER_NAME)
WINZIP_SERVER:=upload/$(WINZIP_SERVER_NAME).win32.zip
$(WINZIP_SERVER_DIR): result.winbuild_32 proto.tag Makefile
	rm -rf $@.dir
	cp -al $<.dir/dist $@.dir
	rm -r $@.dir/$(PACKAGE_NAME).exe $@.dir/*.nsi $@.dir/banner* \
		$@.dir/INSTALL.txt $@.dir/models $@.dir/sound $@.dir/textures \
		$@.dir/SDL* $@.dir/libpng* $@.dir/jpeg*
	echo > $@
$(WINZIP_SERVER): $(WINZIP_SERVER_DIR) upload/.tag
	cd $(WINZIP_SERVER_DIR).dir; zip -9 -r ../../$(WINZIP_SERVER) *
winzip_server.tag: $(WINZIP_SERVER)
	echo > $@

winzip.tag: winzip_server.tag winzip_client.tag winzip_source.tag
	echo > $@


# copy installers into upload
WININST_CLIENT_NAME:=$(PACKAGE_NAME)-$(PACKAGE_VERSION).win32.exe
WININST_SERVER_NAME:=$(PACKAGE_NAME)-dedicated-$(PACKAGE_VERSION).win32.exe
WININST_CLIENT=upload/$(WININST_CLIENT_NAME)
WININST_SERVER=upload/$(WININST_SERVER_NAME)
$(WININST_CLIENT): result.wininst_32 upload/.tag
	rm -f $@
	cp $<.dir/$(WININST_CLIENT_NAME) $@
$(WININST_SERVER): result.wininst_32 upload/.tag
	rm -f $@
	cp $<.dir/$(WININST_SERVER_NAME) $@
windows.tag: $(WININST_CLIENT) $(WININST_SERVER)
	echo > $@

# prerequisites for zeroinstall
zeroinstall.tag: winzip.tag apptar.tag
	touch $@
zeroinstall: zeroinstall.tag

.PHONY: windows winzip_client winzip_server zeroinstall

.PRECIOUS: context.wininst_32 context.winbuild_32 context.winsource_32

#***************************************************************

steamdirs/.tag:
	mkdir -p steamdirs
	echo > $@

# steam packaging
steamdirs/steam_linux: result.appdir_server_steam result.appdir_client_steam steamdirs/.tag
	rm -rf $@.dir
	cp -al $<.dir/appdir $@.dir
	cp -al result.appdir_client_steam.dir/appdir/* $@.dir/
	echo > $@

steamdirs/steam_windows: result.winbuild_steam steamdirs/.tag ${srcdir}/context/steam/windows/*
	rm -rf $@.dir
	cp -al $<.dir/dist $@.dir
	mv $@.dir/*dedicated.exe $@.dir/DEDICATED.hex
	mv $@.dir/*.exe $@.dir/CLIENT.hex
	mv $@.dir/CLIENT.hex $@.dir/$(STEAM_PACKAGE_TITLE).exe
	mv $@.dir/DEDICATED.hex $@.dir/$(STEAM_PACKAGE_TITLE)_Dedicated.exe
	cp -al ${srcdir}/context/steam/windows/* $@.dir/
	rm -rf $@.dir/*.nsi
	rm -rf $@.dir/*.url
	rm -rf $@.dir/banner*
	echo > $@

steamdirs/steam_macos: macOS/steam.tag steamdirs/.tag
	rm -rf $@.dir
	mkdir -p $@.dir
	cd $@.dir; unzip ../../upload/${STEAM_PACKAGE_TITLE}*-client-$(PACKAGE_VERSION).macOS.zip
	mv $@.dir/*.app $@.dir/CLIENT.app
	mv $@.dir/CLIENT.app $@.dir/${STEAM_PACKAGE_TITLE}.app
	echo > $@

steam_linux.tag: steamdirs/steam_linux result.appdir_client_steam result.appdir_server_steam
	echo > $@
steam_windows.tag: steamdirs/steam_windows result.winbuild_steam
	echo > $@
steam_macos.tag: steamdirs/steam_macos
	echo > $@

steam.tag: steam_windows.tag steam_linux.tag steam_macos.tag
	echo > $@
.PHONY: steam steam_windows steam_linux

.PRECIOUS:  context.build_client_steam result.build_client_steam context.appdir_client_steam result.build_server_steam context.build_server_steam context.appdir_server_steam context.winbuild_steam context.winsource_steam

#***************************************************************
# Reformat .md files

context.reformat: rootcontext.64 CI_pack.tag ${srcdir}/context/reformat.sh
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*' upload/*.md ${srcdir}/context/reformat.sh
	rm -f $@.dir/image
	echo "armadeploy_64" > $@.dir/image
	echo > $@
result.reformat: context.reformat
	+$(srcdir)/context_to_result.sh $@.dir ./reformat.sh $*
	echo > $@

reformat.tag: result.reformat
	cp -a $<.dir/*.txt upload/
	cp -a $<.dir/*.html upload/
	echo > $@

#***************************************************************

# for testing, we want minimal builds before any deployment
# in production, the CI solution is responsible for having built
# everything not explicitly required here
deploy_base.tag: CI_pack.tag debtest.tag reformat.tag ../deploy/targets.sh
	echo > $@
deploy_base: deploy_base.tag

# out of our control deploy targets that break from time to time; the whole deploy process needs to be delayed if they fail
deploy_essential: result.deploy_lp result.deploy_scp
deploy_external: deploy_essential deploy_steam

# targets in our control
deploy_ours: result.deploy_zi result.deploy_download deploy_external

context.deploy_base_%: rootcontext.64 deploy_base.tag ${srcdir}/../deploy/deploy_%.sh ${srcdir}/../deploy/wait_for_upload.sh ../deploy/* CI_INFO
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*' upload ../deploy/'*' ${srcdir}/../deploy/deploy_$*.sh ${srcdir}/../deploy/wait_for_upload.sh CI_INFO
	echo > $@
context.deploy_%: context.deploy_base_%
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*'
	echo > $@
result.deploy_%: context.deploy_%
	+$(srcdir)/context_to_result.sh $@.dir ./deploy_$*.sh
	echo > $@

# special case steam: run in steamcmd image
context.deploy_steam: context.deploy_base_steam ${srcdir}/../deploy/steamcontentbuilder/*/* steam.tag deploy_essential
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*' ${srcdir}/../deploy/steamcontentbuilder
	sed -i $@.dir/steamcontentbuilder/scripts/app_build_1306180.vdf -e "s/BUILD_DESCRIPTION/$(STEAM_PACKAGE_TITLE)$(PACKAGE_TITLE_SUFFIX) $(PACKAGE_VERSION)/"
	mkdir -p $@.dir/steamcontentbuilder/content $@.dir/steamcontentbuilder/output
	cp -al steamdirs/steam_windows.dir $@.dir/steamcontentbuilder/content/steam_windows
	cp -al steamdirs/steam_linux.dir $@.dir/steamcontentbuilder/content/steam_linux
	cp -al steamdirs/steam_macos.dir $@.dir/steamcontentbuilder/content/steam_macos
	rm -rf $@.dir/upload
	rm -f $@.dir/image
	echo "steamcmd" > $@.dir/image
	echo > $@

context.deploy_gitlab: rootcontext.64 deploy_base.tag ../deploy/* ${srcdir}/../deploy/deploy_gitlab.sh CI_INFO
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*' upload ../deploy/'*' ${srcdir}/../deploy/deploy_gitlab.sh CI_INFO
	echo "#/bin/bash" > $@.dir/CI_INFO_DEPLOY_GITLAB
	echo "CI_JOB_TOKEN=${CI_JOB_TOKEN}" >> $@.dir/CI_INFO_DEPLOY_GITLAB
	echo "CI_API_V4_URL=${CI_API_V4_URL}" >> $@.dir/CI_INFO_DEPLOY_GITLAB
	echo "CI_PROJECT_ID=${CI_PROJECT_ID}" >> $@.dir/CI_INFO_DEPLOY_GITLAB
	chmod 755 $@.dir/CI_INFO_DEPLOY_GITLAB
	echo > $@

context.deploy_ppa_%: result.debsrc_% ${srcdir}/../deploy/deploy_ppa.sh ../deploy/targets.sh CI_INFO
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*' ${srcdir}/../deploy/deploy_ppa.sh ../deploy/targets.sh ../deploy/secrets/*.gpg CI_INFO
	echo > $@
result.deploy_ppa_%: context.deploy_ppa_% deploy_ours
	+$(srcdir)/context_to_result.sh $@.dir ./deploy_ppa.sh $*
	echo > $@

# deploy for launchpad needs armadeb image
context.deploy_lp: context.deploy_base_lp ${srcdir}/../deploy/lp-project-upload
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*' ${srcdir}/../deploy/lp-project-upload
	rm -f $@.dir/image
	echo "armadeb_64" > $@.dir/image
	echo > $@

context.update_zi: context.deploy_base_zeroinstall ${srcdir}/../deploy/*_zeroinstall.sh
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*' ${srcdir}/../deploy/*_zeroinstall.sh
	rm -f $@.dir/image
	echo "armadeploy_64" > $@.dir/image
	echo > $@
result.update_zi: context.update_zi
	+$(srcdir)/context_to_result.sh $@.dir ./update_zeroinstall.sh
	echo > $@
context.deploy_zi: context.deploy_base_zeroinstall result.update_zi ${srcdir}/../deploy/*_zeroinstall.sh
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*' result.update_zi.dir/zeroinstall ${srcdir}/../deploy/*_zeroinstall.sh
	rm -f $@.dir/image
	echo "armadeploy_64" > $@.dir/image
	echo > $@
result.deploy_zi: context.deploy_zi deploy_external result.deploy_download
	+$(srcdir)/context_to_result.sh $@.dir ./deploy_zeroinstall.sh
	echo > $@

context.deploy_fp: context.deploy_base_flatpak ${srcdir}/../deploy/*_flatpak.sh
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*' ${srcdir}/../deploy/*_flatpak.sh
	rm -f $@.dir/image
	echo "armadeploy_64" > $@.dir/image
	echo > $@
result.deploy_fp: context.deploy_fp deploy_external result.deploy_download
	+$(srcdir)/context_to_result.sh $@.dir ./deploy_flatpak.sh
	echo > $@

context.deploy_itch: context.deploy_base_itch ${srcdir}/../deploy/*_itch.sh upload/$(APPTAR_CLIENT_64) upload/$(APPTAR_CLIENT_32) $(WINZIP_CLIENT) upload/${MACZIP_CLIENT}
	${srcdir}/prepare_context.sh $@.dir $<.dir/'*' ${srcdir}/../deploy/*_itch.sh
	mkdir -p $@.dir/appdir_macos; cd $@.dir/appdir_macos; unzip ../../upload/$(MACZIP_CLIENT); mv ${PACKAGE_TITLE}.app ${PACKAGE_TITLE}" ${PACKAGE_VERSION}".app
	mkdir -p $@.dir/appdir_linux_64; cd $@.dir/appdir_linux_64; tar -xjf ../../upload/$(APPTAR_CLIENT_64)
	mkdir -p $@.dir/appdir_linux_32; cd $@.dir/appdir_linux_32; tar -xjf ../../upload/$(APPTAR_CLIENT_32)
	mkdir -p $@.dir/appdir_windows; cd $@.dir/appdir_windows; unzip ../../$(WINZIP_CLIENT)
	rm -f $@.dir/image
	echo "armadeploy_64" > $@.dir/image
	echo > $@
result.deploy_itch: context.deploy_itch
	+$(srcdir)/context_to_result.sh $@.dir ./deploy_itch.sh
	echo > $@

# deploy to download site (additional dependency here only, standard rules apply)
result.deploy_download: deploy_external

# individual deploy targets
deploy_lp: result.deploy_lp
deploy_scp: result.deploy_scp
deploy_gitlab: result.deploy_gitlab
deploy_steam: result.deploy_steam
deploy_zi: result.deploy_zi
deploy_fp: result.deploy_fp
deploy_download: result.deploy_download
deploy_itch: result.deploy_itch

deploy_ppa: \
deploy_ppa_kinetic \
deploy_ppa_jammy \
deploy_ppa_focal \
deploy_ppa_bionic \
deploy_ppa_xenial \
deploy_ppa_trusty

deploy_ppa_%: result.deploy_ppa_%
	@echo Deployed $*

deploy: deploy_lp deploy_scp deploy_ppa deploy_zi deploy_fp deploy_steam deploy_download deploy_itch
#deploy: deploy_scp
#deploy: deploy_zi
#deploy: deploy_ppa

unstaged_%:
# disable staging, deploy for real
	sed -i ../deploy/targets.sh -e "s/STAGING=true/STAGING=false/"
	STAGING=false MAKE=$(MAKE) ${srcdir}/make_deploy.sh $*

staged_%:
# enforce staging
	sed -i ../deploy/targets.sh -e "s/STAGING=false/STAGING=true/"
	MAKE=$(MAKE) ${srcdir}/make_deploy.sh $*

.PHONY: deploy deploy_ppa deploy_scp deploy_gitlab deploy_steam deploy_base deploy_download

.PRECIOUS: context.debsrc_dapper \
context.debsrc_bionic result.debsrc_bionic context.deploy_ppa_bionic result.deploy_ppa_bionic \
context.debsrc_trusty result.debsrc_trusty context.deploy_ppa_trusty result.deploy_ppa_trusty \
context.debsrc_xenial result.debsrc_xenial context.deploy_ppa_xenial result.deploy_ppa_xenial \
context.debsrc_focal  result.debsrc_focal  context.deploy_ppa_focal  result.deploy_ppa_focal \
context.debsrc_hirsute result.debsrc_hirsute context.deploy_ppa_hirsute result.deploy_ppa_hirsute \
context.debsrc_impish result.debsrc_impish context.deploy_ppa_impish result.deploy_ppa_impish \
context.deploy_base_scp context.deploy_scp \
context.deploy_base_gitlab context.deploy_gitlab \
context.deploy_base_download result.deploy_download context.deploy_download

#***************************************************************

# base64 encode secrets so they can be passed in via a protected configuration variable
../deploy/secrets.b64: ../deploy/secrets/* ../deploy/secrets/*/*
	cd ../deploy; tar --exclude-backups -cjf - secrets | base64 > secrets.b64

secrets: ../deploy/secrets.b64

.PHONY: secrets

#***************************************************************

# test client build on lean alpine linux
test: result.build_client_test result.build_server_test

#***************************************************************

intermediateclean:
	rm -rf context.*

# keep the artifacts clean of junk not needed in the next phase
artifactclean:
	rm -rf \
	*.dir \
	proto/*.dir \
	source

clean: intermediateclean
	rm -rf \
	*.tag \
	gits \
	proto \
	result.* \
	rootcontext.* \
	source \
	steamdirs \
	test.* \
	macOS \
	upload

	if git -C ${srcdir} rev-parse --abbrev-ref HEAD > /dev/null; then rm -f CI_INFO; fi
distclean: clean
	rm -rf $(TARBALL) CI_INFO
	cd ${top_srcdir} && ./bootstrap.sh
	cat ${top_srcdir}/version.m4
