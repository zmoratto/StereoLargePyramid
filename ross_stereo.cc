#include <vw/Image.h>
#include <vw/FileIO.h>
#include <vw/Stereo.h>
#include <vw/Stereo/rewrite/PreFilter.h>
#include <vw/Stereo/rewrite/CorrelationView.h>

using namespace vw;
namespace stereo2 = stereo::rewrite;

int main( int argc, char** argv ) {
  DiskImageView<PixelGray<float> > input1("LLQ2-L.tif"), input2("LLQ2-R.tif");
  DiskImageView<uint8> mask1("LLQ2-lMask.tif"), mask2("LLQ2-rMask.tif");

  Vector2i kernel_size(27,27);
  BBox2i search_volume(Vector2i(300,-90),Vector2i(2475,100));
  //search_volume = search_volume / 4;
  float cross_corr = -1;

  Stopwatch watch;
  watch.start();

  ImageViewRef<PixelMask<Vector2f> > disparity =
    pixel_cast<PixelMask<Vector2f> >(stereo::disparity_mask(stereo2::pyramid_correlate( input1, input2,
                                                                                        stereo2::preprocessing<stereo2::SUBTRACTED_MEAN>(35),
                                                                                        search_volume, kernel_size,
                                                                                        stereo2::CROSS_CORRELATION,
                                                                                        cross_corr), mask1, mask2) );

  boost::scoped_ptr<DiskImageResource> r(DiskImageResource::create("stereo2_disparity.tif",disparity.format()));
  r->set_block_write_size( Vector2i(1024,1024) );
  block_write_image(*r, disparity,
                    TerminalProgressCallback( "", "Rendering: ") );
  watch.stop();

  std::cout << "Run time = " << watch.elapsed_seconds() << "\n";

  return 0;
}
