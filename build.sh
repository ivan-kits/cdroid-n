#处理参数，规范化参数
ARGS=`getopt -a -o p:b:h:: --long product:,build::,help:: -- "$@"`
#echo $ARGS
#将规范化后的命令行参数分配至位置参数（$1,$2,...)
eval set -- "${ARGS}"
TOPDIR=$(dirname "$(readlink -f "$0")")
declare -A TOOLCHAINS #key/value dict ,key is platform,value is toolchain,key must be uppercase
TOOLCHAINS["SIGMA"]=${TOPDIR}/cmake/ssd202-mtitoolchain.cmake
TOOLCHAINS["ALI3528"]=${TOPDIR}/cmake/ali3528-mtitoolchain.cmake
TOOLCHAINS["RISCVD211"]=${TOPDIR}/cmake/riscv64-d211-toolchain.cmake
TOOLCHAINS["R818"]=${TOPDIR}/cmake/r818-toolchain.cmake
TOOLCHAINS["D211"]=${TOPDIR}/cmake/riscv64-d211-toolchain.cmake
TOOLCHAINS["HI3536"]=${TOPDIR}/cmake/hisiv500-toolchain.cmake
TOOLCHAINS["INGENIC"]=${TOPDIR}/cmake/ingenic-x2600-toolchain.cmake

declare -A DEPLIBS #key/value dict,key is platform,value is deplibs dir in vcpkg,key must be uppercase

#VCPKGROOT=/opt/vcpkg
VCPKGROOT=${HOME}/vcpkg

DEPLIBS["X64"]=${VCPKGROOT}/installed/x64-linux-dynamic
DEPLIBS["SIGMA"]=${VCPKGROOT}/installed/arm-linux-dynamic
DEPLIBS["RISCVD211"]=${VCPKGROOT}/installed/riscv64-linux-dynamic
DEPLIBS["R818"]=${VCPKGROOT}/installed/r818-linux
DEPLIBS["D211"]=${VCPKGROOT}/installed/riscv64-d211-linux
DEPLIBS["HI3536"]=${VCPKGROOT}/installed/hisi3536-linux
DEPLIBS["INGENIC"]=${VCPKGROOT}/installed/ingenic-linux

CDROID_VALID_PORTS="x64"
SHOWHELP=0
PRODUCT="x64"
BUILD_TYPE="Release"

for key in "${!TOOLCHAINS[@]}"
do
  CDROID_VALID_PORTS="${CDROID_VALID_PORTS},$key"
done


while :
do
   case $1 in
        -p|--product)
                PRODUCT=$2
                echo "product=$PRODUCT"
                shift
                ;;
        -b|--build)
                BUILD_TYPE=$2
                echo "build=$BUILD_TYPE"
                shift
                ;;
        -h|--help)
                SHOWHELP=1
                echo "showhelp"
                shift
                ;;
        --)
                shift
                break
                ;;
        *)
                break
                ;;
   esac
   shift
done

PORT=${PORT^^}
PRODUCT=${PRODUCT^^}
BUILD_TYPE=${BUILD_TYPE,,}
BUILD_TYPE=${BUILD_TYPE^}
CDROID_DIR=${TOPDIR}/out${PRODUCT}-${BUILD_TYPE}
echo "VALID_PORTS=${CDROID_VALID_PORTS}"
echo "product=$PRODUCT ${PRODUCT,,}"
echo "build=${BUILD_TYPE}/${BUILD_TYPE,,}"

if [ "$PRODUCT" = "X64" ]; then
    echo "x64"
    TOOLCHAIN_FILE=""
elif [ "$PRODUCT" != "X64" ]; then
    TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAINS[${PRODUCT}]}"
    if [ "$TOOLCHAIN_FILE" = "-DCMAKE_TOOLCHAIN_FILE=" ]; then
       SHOWHELP=1
    fi
fi

OUTDIR=out${PRODUCT}-${BUILD_TYPE}
DEPLIBS_DIR=${DEPLIBS[$PRODUCT]}


#Debug version'sDEPLIB seems has some trouble in some platform(r818)
if [ "${BUILD_TYPE,,}" = "debug" ]; then
   DEPLIBS_DIR="${DEPLIBS_DIR}" #/debug:${DEPLIBS_DIR}"
fi

echo "DEPLIBS_DIR=${DEPLIBS_DIR} product=$PRODUCT"
echo "TOOLCHAIN_FILE=${TOOLCHAIN_FILE} SHOWHELP=${SHOWHELP}"
echo "========DEPLIBS_DIR=${DEPLIBS_DIR} BUILDTYPE=${BUILD_TYPE}"
export PATH=$DEPLIBS_DIR:$PATH

if [ $SHOWHELP -gt 0 ] ;then
    echo "Usage: $0 [options] $#"
    echo "-P|--product [${CDROID_VALID_PORTS}] default is x64"
    echo "-b|--build[Debug,Release,RelWithDebInfo,MinSizeRel]"
    echo "-h|--help Show Help Info,Usage"
    echo ""
    exit
fi

mkdir -p ${OUTDIR}
pushd ${OUTDIR}

export PKG_CONFIG_PATH=$DEPLIBS_DIR/lib/pkgconfig
export PKG_CONFIG_LIBDIR=$DEPLIBS_DIR/lib/pkgconfig
echo PKG_CONFIG_PATH=${PKG_CONFIG_PATH}
cmake ${TOOLCHAIN_FILE} \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DCMAKE_PREFIX_PATH=${DEPLIBS_DIR} \
   -DCMAKE_MODULE_PATH=${DEPLIBS_DIR} \
   -DCDROID_CHIPSET=${PRODUCT,,} \
   -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
   ..
