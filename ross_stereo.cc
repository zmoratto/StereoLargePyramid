#include <vw/Image.h>
#include <vw/FileIO.h>
#include <vw/Stereo.h>
#include <vw/Stereo/rewrite/PreFilter.h>
#include <vw/Stereo/rewrite/CorrelationView.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

using namespace vw;
namespace stereo2 = stereo::rewrite;
namespace fs = boost::filesystem;

template <class ImageT>
void block_write_image( const std::string& filename,
                        ImageViewBase<ImageT> const& image,
                        const ProgressCallback& progress_callback = ProgressCallback::dummy_instance() ) {
  boost::scoped_ptr<DiskImageResource> r( DiskImageResource::create( filename, image.format() ) );

  if ( r->has_block_write() ) {
    r->set_block_write_size( Vector2i(vw_settings().default_tile_size(),
                                      vw_settings().default_tile_size()) );
    block_write_image( *r, image.impl(), progress_callback );
  } else {
    write_image( *r, image.impl(), progress_callback );
  }
}

int main( int argc, char** argv ) {
  DiskImageView<PixelGray<float> > input1("LLQ2-L.tif"), input2("LLQ2-R.tif");

  Vector2i kernel_size(27,27);
  BBox2i search_volume(Vector2i(300,-90),Vector2i(2475,100));
  float cross_corr = -1;

  if (0) {
    DiskImageView<uint8> mask1("LLQ2-lMask.tif"), mask2("LLQ2-rMask.tif");

    // This is stereo just using our standard per tile correlation.
    Stopwatch watch;
    watch.start();

    ImageViewRef<PixelMask<Vector2i> > disparity =
      stereo::disparity_mask(stereo2::pyramid_correlate( input1, input2,
                                                         stereo2::preprocessing<stereo2::SUBTRACTED_MEAN>(35),
                                                         search_volume, kernel_size,
                                                         stereo2::CROSS_CORRELATION,
                                                         cross_corr), mask1, mask2);

    boost::scoped_ptr<DiskImageResource> r(DiskImageResource::create("stereo2_disparity.tif",disparity.format()));
    r->set_block_write_size( Vector2i(1024,1024) );
    block_write_image(*r, disparity,
                      TerminalProgressCallback( "", "Rendering: ") );
    watch.stop();

    std::cout << "Run time = " << watch.elapsed_seconds() << "\n";
  }

  {
    // This is a global pyramid method
    std::vector<DefaultKernelT<PixelGray<float> >::type > kernel(5);
    kernel[0] = kernel[4] = 1.0/16.0;
    kernel[1] = kernel[3] = 4.0/16.0;
    kernel[2] = 6.0/16.0;

    int32 max_pyramid_levels = 5;
    fs::create_symlink("LLQ2-L.tif", "L0.tif");
    fs::create_symlink("LLQ2-R.tif", "R0.tif");
    // Building pyramids
    for ( int32 i = 0; i < max_pyramid_levels; i++ ) {
      std::string num = boost::lexical_cast<std::string>(i);
      std::string nnum = boost::lexical_cast<std::string>(i+1);
      block_write_image("L"+nnum+".tif",
                        subsample(separable_convolution_filter(DiskImageView<PixelGray<float> >("L"+num+".tif"),
                                                               kernel, kernel),2) );
      block_write_image("R"+nnum+".tif",
                        subsample(separable_convolution_filter(DiskImageView<PixelGray<float> >("R"+num+".tif"),
                                                               kernel, kernel),2) );
    }

    { // Start out the correlation
      std::string num = boost::lexical_cast<std::string>(max_pyramid_levels);
      DiskImageView<PixelGray<float> > left("L"+num+".tif"), righ("R"+num+".tif");
      ImageViewRef<PixelMask<Vector2i> > disparity =
        pyramid_correlate( left, righ,
                           stereo2::preprocessing<stereo2::SUBTRACTED_MEAN>(35),
                           search_volume / (0x01 << max_pyramid_levels),kernel_size,
                           stereo2::CROSS_CORRELATION, cross_corr);

      block_write_image( "D0.tif", disparity, TerminalProgressCallback("", "D0.tif") );
    }

    // Do the other layers
    for ( int32 i = max_pyramid_levels - 1; i >= 0; i-- ) {
      std::string num = boost::lexical_cast<std::string>(i);
      DiskImageView<PixelGray<float> > left("L"+num+".tif"), righ("R"+num+".tif");
    }
  }

  return 0;
}
