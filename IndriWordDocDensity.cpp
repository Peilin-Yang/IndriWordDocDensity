#include "indri/Repository.hpp"
#include "indri/CompressedCollection.hpp"
#include "indri/LocalQueryServer.hpp"
#include "indri/ScopedLock.hpp"
#include "indri/QueryEnvironment.hpp"
#include "indri/Parameters.hpp"

#include <iostream>
#include <vector>


void print_word_doc_density(indri::collection::Repository& r, const std::string& term, bool debug=false) {
    std::string stem = r.processTerm( term );
    indri::server::LocalQueryServer local(r);
    
    UINT64 df = local.documentCount( stem );
    double dd = 0.0;
    if (debug) {
        std::cout << term << " "
                  << stem << " "
                  << df << " " << std::endl;
    }
    
    indri::collection::Repository::index_state state = r.indexes();
    
    for( size_t i=0; i<state->size(); i++ ) {
      indri::index::Index* index = (*state)[i];
      indri::thread::ScopedLock( index->iteratorLock() );
    
      indri::index::DocListIterator* iter = index->docListIterator( stem );
      if (iter == NULL) continue;
    
      iter->startIteration();
    
      int doc = 0;
      indri::index::DocListIterator::DocumentData* entry;
    
      for( iter->startIteration(); iter->finished() == false; iter->nextEntry() ) {
        entry = iter->currentEntry();
        double this_tf = entry->positions.size();
        double this_doc_len = index->documentLength( entry->document );
        dd += this_tf*1.0 / this_doc_len;
        if (debug) {
            std::cout << this_tf << " "
                      << this_doc_len << " "
                      << dd << std::endl;
        }
      }
    
      delete iter;
    }

    dd /= df;
    std::cout << dd << std::endl;
}

void usage() {
    std::cout << "    IndriTextTransformer -index=<index path> -term=<input term> [-debug=0]" << std::endl;
}

void require_parameter( const char* name, indri::api::Parameters& p ) {
  if( !p.exists( name ) ) {
    usage();
    LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must specify a " + name + " parameter." );
  }
}


int main( int argc, char** argv ) {
  try {
    indri::api::Parameters& parameters = indri::api::Parameters::instance();
    parameters.loadCommandLine( argc, argv );

    require_parameter( "index", parameters );
    require_parameter( "term", parameters );

    bool debug = false;
    if ( parameters.get( "debug", "" ) != "") {
      std::string debug_str = parameters.get( "debug", "" );
      if ( debug_str != "0" ) {
        debug = true;
      } 
    }

    indri::collection::Repository r;
    std::string repName = parameters["index"];
    std::string term = parameters["term"];
    
    r.openRead( repName );
    print_word_doc_density(r, term, debug);
    r.close();
    return 0;
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  }
}


