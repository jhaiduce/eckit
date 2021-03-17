/*
 * (C) Copyright 2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include <cstring>

#include "atlas/array/Array.h"
#include "atlas/array/ArrayView.h"

#include "atlas/io/atlas-io.h"
#include "atlas/io/types/array/ArrayReference.h"
#include "tests/AtlasTestEnvironment.h"

#include "atlas/io/types/array/adaptors/ArrayAdaptor.h"

namespace atlas {
namespace test {

using io::ArrayReference;

// -------------------------------------------------------------------------------------------------------

struct UnencodableType {
    std::string _;
};

// -------------------------------------------------------------------------------------------------------

// Example type that can be encoded / decoded with atlas::io
// The "operations" performed on this type are stored for unit-test purposes
class EncodableType {
public:
    using Operations = std::vector<std::string>;

    EncodableType( std::string s, std::shared_ptr<Operations> operations = std::make_shared<Operations>() ) :
        str( s ), ops( operations ) {
        ATLAS_TRACE( "EncodableType[" + str + "] construct" );
        ops->push_back( "constructor" );
    }

    EncodableType() : ops( std::make_shared<Operations>() ) {}

    EncodableType( const EncodableType& other ) {
        // This constructor should not be called.
        str = other.str;
        ATLAS_TRACE( "EncodableType[" + str + "] copy constructor" );
        ops = other.ops;
        ops->push_back( "copy constructor" );
    }

    EncodableType( EncodableType&& other ) {
        str = std::move( other.str );
        ATLAS_TRACE( "EncodableType[" + str + "] move constructor" );
        ops = other.ops;
        ops->push_back( "move constructor" );
    }


    EncodableType& operator=( const EncodableType& rhs ) {
        // This assignment should not be called.
        str = rhs.str;
        ATLAS_TRACE( "EncodableType[" + str + "] assignment" );
        ops = rhs.ops;
        ops->push_back( "assignment" );
        return *this;
    }

    EncodableType& operator=( const EncodableType&& rhs ) {
        // This assignment should not be called.
        str = std::move( rhs.str );
        ATLAS_TRACE( "EncodableType[" + str + "] move" );
        ops = rhs.ops;
        ops->push_back( "move" );
        return *this;
    }


    friend void encode_data( const EncodableType& in, atlas::io::Data& out ) {
        in.ops->push_back( "encode_data" );
        out.assign( in.str.data(), in.str.size() );
    }

    friend size_t encode_metadata( const EncodableType& in, atlas::io::Metadata& metadata ) {
        in.ops->push_back( "encode_metadata" );
        metadata.set( "type", "EncodableType" );
        metadata.set( "bytes", in.str.size() );
        return in.str.size();
    }

    friend void decode( const atlas::io::Metadata&, const atlas::io::Data& b, EncodableType& out ) {
        out.ops->push_back( "decode" );
        const char* data = static_cast<const char*>( b.data() );
        out.str          = std::string( data, data + b.size() );
    }

    const std::vector<std::string>& operations() const { return *ops; }

private:
    std::string str;
    mutable std::shared_ptr<std::vector<std::string>> ops;
};

// -------------------------------------------------------------------------------------------------------

CASE( "test exceptions" ) {
    EXPECT( not( io::is_interpretable<UnencodableType, ArrayReference>() ) );
    EXPECT( not io::is_encodable<UnencodableType>() );
    EXPECT( not io::can_encode_metadata<UnencodableType>() );
    EXPECT( not io::can_encode_data<UnencodableType>() );

    UnencodableType in;
    atlas::io::Data data;
    atlas::io::Metadata metadata;

    EXPECT_THROWS_AS( io::ref( in, io::tag::disable_static_assert() ), io::NotEncodable );
    EXPECT_THROWS_AS( io::copy( in, io::tag::disable_static_assert() ), io::NotEncodable );
    EXPECT_THROWS_AS( io::encode( in, metadata, data, io::tag::disable_static_assert() ), io::NotEncodable );
}

// -------------------------------------------------------------------------------------------------------

CASE( "encoding test::EncodableType" ) {
    static_assert( not io::is_interpretable<EncodableType, ArrayReference>(), "" );
    static_assert( io::is_encodable<EncodableType>(), "" );
    static_assert( io::is_decodable<EncodableType>(), "" );

    const std::string encoded_string{"encoded string"};
    EncodableType in( encoded_string );
    atlas::io::Data data;
    atlas::io::Metadata metadata;
    EXPECT_NO_THROW( encode( in, metadata, data ) );

    EXPECT( metadata.type() == "EncodableType" );
    EXPECT( data.size() == encoded_string.size() );
    EXPECT( ::memcmp( data, encoded_string.data(), encoded_string.size() ) == 0 );
}


// -------------------------------------------------------------------------------------------------------

CASE( "encoding atlas::io::types::ArrayView" ) {
    static_assert( not io::is_interpretable<ArrayReference, ArrayReference>(), "" );
    static_assert( io::can_encode_data<ArrayReference>(), "" );
    static_assert( io::can_encode_metadata<ArrayReference>(), "" );
    static_assert( io::is_encodable<ArrayReference>(), "" );
}

// -------------------------------------------------------------------------------------------------------

template <typename T>
void assert_Vector() {
    static_assert( not io::is_interpretable<atlas::vector<T>, ArrayReference>(), "" );
    static_assert( io::can_encode_data<atlas::vector<T>>(), "" );
    static_assert( io::can_encode_metadata<atlas::vector<T>>(), "" );
    static_assert( io::is_encodable<atlas::vector<T>>(), "" );
}

CASE( "encoding atlas::vector" ) {
    assert_Vector<int>();
    assert_Vector<float>();
    assert_Vector<double>();
    assert_Vector<long>();

    atlas::vector<double> in{1, 2, 3, 4, 5};
    atlas::io::Data data;
    atlas::io::Metadata metadata;
    EXPECT_NO_THROW( encode( in, metadata, data ) );

    using T = double;
    EXPECT( data.size() == size_t( in.size() ) * sizeof( T ) );
    EXPECT( ::memcmp( in.data(), data.data(), data.size() ) == 0 );
    EXPECT( metadata.type() == "array" );
    EXPECT( metadata.getString( "datatype" ) == atlas::array::DataType::str<T>() );
}

// -------------------------------------------------------------------------------------------------------

template <typename T>
void assert_StdVector() {
    static_assert( io::is_interpretable<std::vector<T>, ArrayReference>(), "" );
    static_assert( not io::can_encode_data<std::vector<T>>(), "" );
    static_assert( not io::can_encode_metadata<std::vector<T>>(), "" );
    static_assert( not io::is_encodable<std::vector<T>>(), "" );
}

template <typename T>
void encode_StdVector() {
    std::vector<T> in{1, 2, 3, 4, 5};

    ArrayReference interpreted;
    interprete( in, interpreted );

    atlas::io::Data data;
    atlas::io::Metadata metadata;

    encode( interpreted, metadata, data );

    EXPECT( data.size() == in.size() * sizeof( T ) );
    EXPECT( ::memcmp( in.data(), data.data(), data.size() ) == 0 );
    EXPECT( metadata.type() == "array" );
    EXPECT( metadata.getString( "datatype" ) == atlas::array::DataType::str<T>() );
}

CASE( "encoding std::vector" ) {
    assert_StdVector<int>();
    assert_StdVector<float>();
    assert_StdVector<double>();
    assert_StdVector<long>();

    encode_StdVector<int>();
    encode_StdVector<float>();
    encode_StdVector<double>();
    encode_StdVector<long>();
}

// -------------------------------------------------------------------------------------------------------

CASE( "encoding atlas::array::Array" ) {
    static_assert( io::is_interpretable<atlas::array::Array, ArrayReference>(), "" );
    static_assert( not io::can_encode_data<atlas::array::Array>(), "" );
    static_assert( not io::can_encode_metadata<atlas::array::Array>(), "" );
    static_assert( not io::is_encodable<atlas::array::Array>(), "" );

    using T = double;
    array::ArrayT<T> in( 4, 2 );
    array::make_view<T, 2>( in ).assign( {1, 2, 3, 4, 5, 6, 7, 8} );

    {
        auto interpreted = io::interprete<ArrayReference>( in );
        EXPECT( interpreted.datatype() == in.datatype() );
        EXPECT_EQ( interpreted.rank(), 2 );
        EXPECT_EQ( interpreted.shape( 0 ), 4 );
        EXPECT_EQ( interpreted.shape( 1 ), 2 );
        EXPECT_EQ( interpreted.size(), in.size() );
    }

    atlas::io::Data data;
    atlas::io::Metadata metadata;
    EXPECT_NO_THROW( encode( in, metadata, data ) );

    io::ArrayMetadata array_metadata{metadata};
    EXPECT( array_metadata.datatype() == in.datatype() );
    EXPECT_EQ( array_metadata.rank(), 2 );
    EXPECT_EQ( array_metadata.shape( 0 ), 4 );
    EXPECT_EQ( array_metadata.shape( 1 ), 2 );
    EXPECT_EQ( array_metadata.size(), in.size() );

    EXPECT_EQ( data.size(), in.size() * in.datatype().size() );
    EXPECT( ::memcmp( in.data(), data, data.size() ) == 0 );
    EXPECT( metadata.type() == "array" );
    EXPECT( metadata.getString( "datatype" ) == in.datatype().str() );
    EXPECT( metadata.getIntVector( "shape" ) == in.shape() );
}

// -------------------------------------------------------------------------------------------------------

CASE( "test Encoder" ) {
    SECTION( "default constructor" ) {
        io::Encoder encoder;
        EXPECT( encoder == false );
        io::Metadata metadata;
        io::Data data;
        EXPECT_THROWS_AS( encode( encoder, metadata, data ), eckit::AssertionFailed );
    }

    SECTION( "Encoder via reference" ) {
        io::Encoder encoder;
        auto ops = std::make_shared<EncodableType::Operations>();

        EncodableType encodable( "string", ops );
        EXPECT_EQ( ops->size(), 1 );
        EXPECT_EQ( ops->back(), "constructor" );

        io::ref( encodable );
        EXPECT_EQ( ops->size(), 1 );
        EXPECT_EQ( ops->back(), "constructor" );

        encoder = io::Encoder{io::ref( encodable )};
        EXPECT_EQ( ops->size(), 2 );
        EXPECT_EQ( ops->back(), "encode_metadata" );

        io::Metadata metadata;
        io::Data data;
        encode( encoder, metadata, data );
        EXPECT_EQ( ops->size(), 3 );
        EXPECT_EQ( ops->back(), "encode_data" );
    }

    SECTION( "Encoder via copy" ) {
        io::Encoder encoder;
        auto ops = std::make_shared<EncodableType::Operations>();

        EncodableType encodable( "string", ops );
        EXPECT_EQ( ops->size(), 1 );
        EXPECT_EQ( ops->back(), "constructor" );


        encoder = io::Encoder{io::copy( encodable )};
        EXPECT_EQ( ops->size(), 3 );
        EXPECT_EQ( ops->at( 1 ), "encode_metadata" );
        EXPECT_EQ( ops->at( 2 ), "encode_data" );

        io::Metadata metadata;
        io::Data data;
        encode( encoder, metadata, data );
        EXPECT_EQ( ops->size(), 3 );
        EXPECT_EQ( ops->at( 2 ), "encode_data" );
    }

    SECTION( "Encoder via move" ) {
        io::Encoder encoder;
        auto ops = std::make_shared<EncodableType::Operations>();

        EncodableType encodable( "string", ops );
        EXPECT_EQ( ops->size(), 1 );
        EXPECT_EQ( ops->back(), "constructor" );

        encoder = io::Encoder{std::move( encodable )};
        EXPECT_EQ( ops->size(), 3 );
        EXPECT_EQ( ops->at( 1 ), "move constructor" );
        EXPECT_EQ( ops->at( 2 ), "encode_metadata" );

        io::Metadata metadata;
        io::Data data;
        encode( encoder, metadata, data );
        EXPECT_EQ( ops->size(), 4 );
        EXPECT_EQ( ops->at( 3 ), "encode_data" );
    }
}

// -------------------------------------------------------------------------------------------------------

CASE( "Encoder for std::vector" ) {
    SECTION( "ref" ) {
        using T = double;
        std::vector<T> v{1, 2, 3, 4, 5, 6, 7, 8};

        io::Encoder encoder( io::ref( v ) );

        // We can only encode with reference to original vector (no copies were made)
        io::Metadata metadata;
        io::Data data;
        encode( encoder, metadata, data );
        EXPECT( data.size() == v.size() * sizeof( T ) );
        EXPECT( ::memcmp( data, v.data(), data.size() ) == 0 );
    }

    SECTION( "copy" ) {
        using T = double;
        std::vector<T> v{1, 2, 3, 4, 5, 6, 7, 8};

        io::Encoder encoder;
        {
            std::vector<T> scoped = v;
            encoder               = io::Encoder( io::copy( scoped ) );
            scoped.assign( scoped.size(), 0 );  // zero out before destruction
        }

        // We can now encode with scoped vector destroyed
        io::Metadata metadata;
        io::Data data;
        encode( encoder, metadata, data );
        EXPECT_EQ( data.size(), v.size() * sizeof( T ) );
        EXPECT( ::memcmp( data, v.data(), data.size() ) == 0 );
    }
}

// -------------------------------------------------------------------------------------------------------

CASE( "Encoder for atlas::vector" ) {
    SECTION( "ref" ) {
        using T = double;
        atlas::vector<T> v{1, 2, 3, 4, 5, 6, 7, 8};

        io::Encoder encoder( io::ref( v ) );

        // We can only encode with reference to original vector (no copies were made)
        io::Metadata metadata;
        io::Data data;
        encode( encoder, metadata, data );
        EXPECT_EQ( data.size(), size_t( v.size() ) * sizeof( T ) );
        EXPECT( ::memcmp( data, v.data(), data.size() ) == 0 );
    }

    SECTION( "copy" ) {
        using T = double;
        atlas::vector<T> v{1, 2, 3, 4, 5, 6, 7, 8};

        io::Encoder encoder;
        {
            atlas::vector<T> scoped = v;
            encoder                 = io::Encoder( io::copy( scoped ) );
            scoped.assign( scoped.size(), 0 );  // zero out before destruction
        }

        // We can now encode with scoped vector destroyed
        io::Metadata metadata;
        io::Data data;
        encode( encoder, metadata, data );
        EXPECT_EQ( data.size(), size_t( v.size() ) * sizeof( T ) );
        EXPECT( ::memcmp( data, v.data(), data.size() ) == 0 );
    }
}

// -------------------------------------------------------------------------------------------------------

CASE( "Encoder for atlas::array::Array" ) {
    SECTION( "ref" ) {
        using T = double;
        array::ArrayT<T> v( 4, 2 );
        array::make_view<T, 2>( v ).assign( {1, 2, 3, 4, 5, 6, 7, 8} );

        io::Encoder encoder( io::ref( v ) );

        // We can only encode with reference to original vector (no copies were made)
        io::Metadata metadata;
        io::Data data;
        encode( encoder, metadata, data );
        EXPECT_EQ( data.size(), v.size() * sizeof( T ) );
        EXPECT( ::memcmp( data, v.data(), data.size() ) == 0 );
    }

    SECTION( "copy" ) {
        using T = double;
        array::ArrayT<T> v( 4, 2 );
        array::make_view<T, 2>( v ).assign( {1, 2, 3, 4, 5, 6, 7, 8} );

        io::Encoder encoder;
        {
            array::ArrayT<T> scoped( 4, 2 );
            ::memcpy( scoped.data(), v.data(), v.size() * sizeof( T ) );
            encoder = io::Encoder( io::copy( v ) );
            array::make_view<T, 2>( scoped ).assign( 0 );
        }
        // We can now encode with scoped vector destroyed
        io::Metadata metadata;
        io::Data data;
        encode( encoder, metadata, data );
        EXPECT_EQ( data.size(), v.size() * sizeof( T ) );
        EXPECT( ::memcmp( data, v.data(), data.size() ) == 0 );
    }
}


CASE( "Encoder of encoder" ) {
    using T = double;
    array::ArrayT<T> v( 4, 2 );
    array::make_view<T, 2>( v ).assign( {1, 2, 3, 4, 5, 6, 7, 8} );

    io::Encoder encoder( io::ref( v ) );
    io::Encoder encoder_of_encoder( io::ref( encoder ) );

    io::Metadata metadata;
    io::Data data;
    encode( encoder_of_encoder, metadata, data );
    EXPECT_EQ( data.size(), v.size() * sizeof( T ) );
    EXPECT( ::memcmp( data, v.data(), data.size() ) == 0 );
}

// -------------------------------------------------------------------------------------------------------

/// Helper class to be used in testing decoding of arrays.
template <typename T>
struct EncodedArray {
    atlas::io::Data data;
    atlas::io::Metadata metadata;

    EncodedArray() : in( 4, 2 ) {
        array::make_view<double, 2>( in ).assign( {1, 2, 3, 4, 5, 6, 7, 8} );
        encode( in, metadata, data );
    }

    friend bool operator==( const std::vector<T>& lhs, const EncodedArray<T>& rhs ) {
        if ( lhs.size() != rhs.in.size() ) {
            return false;
        }
        return ::memcmp( lhs.data(), rhs.in.data(), rhs.in.size() * rhs.in.datatype().size() ) == 0;
    }
    friend bool operator==( const atlas::vector<T>& lhs, const EncodedArray<T>& rhs ) {
        if ( size_t( lhs.size() ) != rhs.in.size() ) {
            return false;
        }
        return ::memcmp( lhs.data(), rhs.in.data(), rhs.in.size() * rhs.in.datatype().size() ) == 0;
    }
    friend bool operator==( const atlas::array::Array& lhs, const EncodedArray<T>& rhs ) {
        if ( lhs.datatype() != rhs.in.datatype() ) {
            return false;
        }
        if ( lhs.size() != rhs.in.size() ) {
            return false;
        }
        return ::memcmp( lhs.data(), rhs.in.data(), rhs.in.size() * rhs.in.datatype().size() ) == 0;
    }

private:
    atlas::array::ArrayT<T> in;
};

// -------------------------------------------------------------------------------------------------------

CASE( "Decoding to std::vector" ) {
    using T = double;
    EncodedArray<T> encoded;
    std::vector<T> out;

    SECTION( "decode std::vector directly" ) {
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, out ) );
        EXPECT( out == encoded );
    }

    SECTION( "decode using rvalue io::Decoder (type erasure)" ) {
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, io::Decoder( out ) ) );
        EXPECT( out == encoded );
    }

    SECTION( "decode using lvalue io::Decoder (type erasure)" ) {
        io::Decoder decoder( out );
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, io::Decoder( out ) ) );
        EXPECT( out == encoded );
    }

    SECTION( "decode using decoder of decoder" ) {
        io::Decoder decoder( out );
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, io::Decoder( decoder ) ) );
        EXPECT( out == encoded );
    }
}

// -------------------------------------------------------------------------------------------------------

CASE( "Decoding to atlas::vector" ) {
    using T = double;
    EncodedArray<T> encoded;
    atlas::vector<T> out;

    SECTION( "decode directly" ) {
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, out ) );
        EXPECT( out == encoded );
    }

    SECTION( "decode using rvalue io::Decoder (type erasure)" ) {
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, io::Decoder( out ) ) );
        EXPECT( out == encoded );
    }

    SECTION( "decode using lvalue io::Decoder (type erasure)" ) {
        io::Decoder decoder( out );
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, io::Decoder( out ) ) );
        EXPECT( out == encoded );
    }

    SECTION( "decode using decoder of decoder" ) {
        io::Decoder decoder( out );
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, io::Decoder( decoder ) ) );
        EXPECT( out == encoded );
    }
}

// -------------------------------------------------------------------------------------------------------

CASE( "Decoding to atlas::array::Array" ) {
    using T = double;
    EncodedArray<T> encoded;
    atlas::array::ArrayT<T> out( 0, 0 );

    SECTION( "decode directly" ) {
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, out ) );
        EXPECT( out == encoded );
    }

    SECTION( "decode using rvalue io::Decoder (type erasure)" ) {
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, io::Decoder( out ) ) );
        EXPECT( out == encoded );
    }

    SECTION( "decode using lvalue io::Decoder (type erasure)" ) {
        io::Decoder decoder( out );
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, io::Decoder( out ) ) );
        EXPECT( out == encoded );
    }

    SECTION( "decode using decoder of decoder" ) {
        io::Decoder decoder( out );
        EXPECT_NO_THROW( decode( encoded.metadata, encoded.data, io::Decoder( decoder ) ) );
        EXPECT( out == encoded );
    }
}

// -------------------------------------------------------------------------------------------------------

}  // namespace test
}  // namespace atlas

int main( int argc, char** argv ) {
    return atlas::test::run( argc, argv );
}
