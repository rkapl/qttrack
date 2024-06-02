pkgname=qttrack
pkgver=1.0
pkgrel=1
pkgdesc=""
arch=(any)
url="http://github.com/rkapl/qttrack"
license=('GPL')
depends=(qt6-base libical)
makedepends=(qt6-base libical)
source=()
md5sums=()

prepare() {
   :
}

build() {
   mkdir -p build
   cd build
   qmake6 ../.. DEFINES+=TESTS
	make
}

check() {
   builddir="`pwd`/build"
   cd ../tests
   "${builddir}/qttrack" --tests
}

package() {
	cd build
	make INSTALL_ROOT="$pkgdir" install
}
