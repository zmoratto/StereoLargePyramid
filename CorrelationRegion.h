#ifndef __CORRELATION_REGION_H__
#define __CORRELATION_REGION_H__

#include "SubDivideRegions.h"
#include <vw/Stereo/Rewrite/CorrelationView.h>

namespace vw {
namespace stereo {
namespace rewrite {

  /// An Image View that correlates regions with different settings
  template <class Image1T, class Image2T, class PreFilterT>
  class CorrelationByRegionView : public ImageViewBase<CorrelationByRegionView<Image1T,Image2T,PreFilterT> > {

    Image1T m_left_image;
    Image2T m_right_image;
    PreFilterT m_prefilter;
    std::list<SearchParam> m_search_params;
    Vector2i m_kernel_size;
    rewrite::CostFunctionType m_cost_type;
    float m_consistency_threshold;

  public:
    typedef PixelMask<Vector2i> pixel_type;
    typedef PixelMask<Vector2i> result_type;
    typedef ProceduralPixelAccessor<CorrelationByRegionView> pixel_accessor;

    CorrelationByRegionView( ImageViewBase<Image1T> const& left,
                             ImageViewBase<Image2T> const& right,
                             PreFilterT const& prefilter,
                             std::list<SearchParam> const& search_params,
                             Vector2i const& kernel_size,
                             CostFunctionType cost_type = ABSOLUTE_DIFFERENCE,
                             float consistency_threshold = -1 ) :
      m_left_image(left.impl()), m_right_image(right.impl()), 
      m_prefilter(prefilter), m_search_params(search_params), m_kernel_size(kernel_size),
      m_cost_type(cost_type), m_consistency_threshold(consistency_threshold) {
    }

    // Standard required ImageView interfaces
    inline int32 cols() const { return m_left_image.cols(); }
    inline int32 rows() const { return m_left_image.rows(); }
    inline int32 planes() const { return 1; }

    inline pixel_accessor origin() const { return pixel_accessor( *this, 0, 0 ); }
    inline pixel_type operator()( int32 /*i*/, int32 /*j*/, int32 /*p*/ = 0) const {
      vw_throw( NoImplErr() << "CorrelationView::operator()(....) has not been implemented." );
      return pixel_type();
    }

    // Block rasterization section that does actual work
    typedef CropView<ImageView<pixel_type> > prerasterize_type;
    inline prerasterize_type prerasterize(BBox2i const& bbox) const {
      ImageView<pixel_type> result( bbox.width(), bbox.height() );
      BOOST_FOREACH( SearchParam const& param, m_search_params ) {
        if ( bbox.intersects(param.first) ) {
          BBox2i global_bbox = bbox;
          global_bbox.crop( param.first );
          BBox2i local_bbox  = global_bbox - bbox.min();
          crop(result,local_bbox) =
            crop(correlate(m_left_image,m_right_image,
                           m_prefilter,param.second,m_kernel_size,
                           m_cost_type,
                           m_consistency_threshold),
                 global_bbox);
        }
      }
      return prerasterize_type(result,
                               -bbox.min().x(), -bbox.min().y(),
                               cols(), rows());
    }
    template <class DestT>
    inline void rasterize(DestT const& dest, BBox2i const& bbox) const {
      vw::rasterize(prerasterize(bbox), dest, bbox);
    }
  };

  template <class Image1T, class Image2T, class PreFilterT>
  CorrelationByRegionView<Image1T,Image2T,PreFilterT>
  correlate( ImageViewBase<Image1T> const& left,
             ImageViewBase<Image2T> const& right,
             PreFilterT const& filter,
             std::list<SearchParam> const& search_params,
             Vector2i const& kernel_size,
             CostFunctionType cost_type = ABSOLUTE_DIFFERENCE,
             float consistency_threshold = -1 ) {
    typedef CorrelationByRegionView<Image1T,Image2T,PreFilterT> result_type;
    return result_type( left.impl(), right.impl(), filter, search_params,
                        kernel_size, cost_type, consistency_threshold );
  }

}}} // end vw::stereo::rewrite

#endif//__CORRELATION_REGION_H__
