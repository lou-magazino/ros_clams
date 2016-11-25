#include <core/clams/trajectory.h>
#include <core/stream_sequence/stream_sequence_base.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

using namespace std;
namespace bfs = boost::filesystem;
using namespace Eigen;
using namespace clams;

int main(int argc, char** argv)
{
  namespace bpo = boost::program_options;
  bpo::options_description opts_desc("Allowed options");
  bpo::positional_options_description p;

  string sseq_path;
  string src;
  string dst;
  opts_desc.add_options()
    ("help,h", "produce help message")
    ("sseq", bpo::value(&sseq_path)->required(), "StreamSequence")
    ("src", bpo::value(&src)->required(), "Freiburg trajectory file")
    ("dst", bpo::value(&dst)->required(), "Clams trajectory file")
    ;

  p.add("sseq", 1);
  p.add("src", 1);
  p.add("dst", 1);

  bpo::variables_map opts;
  bpo::store(bpo::command_line_parser(argc, argv).options(opts_desc).positional(p).run(), opts);
  bool badargs = false;
  try { bpo::notify(opts); }
  catch(...) { badargs = true; }
  if(opts.count("help") || badargs) {
    cout << "Usage: " << argv[0] << " [OPTS] SSEQ SRC DST" << endl;
    cout << endl;
    cout << opts_desc << endl;
    return 1;
  }

  // StreamSequenceBase::Ptr sseq = StreamSequenceBase::initializeFromDirectory(sseq_path);
  Trajectory traj;
  // traj.resize(sseq->size());
  if(!bfs::is_regular_file(bfs::path(sseq_path + "/assoc.txt")))
  {
    ROS_ERROR("assoc.txt not found");
    return 1;
  }
  
  ifstream frei;
  ifstream assoc;
  frei.open(src.c_str());
  assoc.open((sseq_path + "/assoc.txt").c_str());
  // read all timestamp from assoc
  vector<double> timestamps;
  while(true) {
    double t1, t2;
    std::string f1, f2;
    assoc >> t1 >> f1 >> t2 >> f2;
    if(assoc.eof())
      break;
    // cout << t1 << f1 << t2 << f2 << "\n";
    timestamps.push_back(t1);
  }
  assoc.close();
  traj.resize(timestamps.size());
  while(true) {
    double timestamp, tx, ty, tz, qx, qy, qz, qw;
    frei >> timestamp >> tx >> ty >> tz >> qx >> qy >> qz >> qw;
    if(frei.eof())
      break;
    
    double dt = timestamp;
    // size_t idx = sseq->seek(timestamp, &dt);
    size_t idx = 0;
    for(size_t i = 0; i < timestamps.size(); ++i)
    {
      if(fabs(timestamps.at(i) - timestamp) < dt)
      {
	idx = i;
	dt = fabs(timestamps.at(i) - timestamp);
      }
    }
    
    ROS_ASSERT(dt < 1e-6);

    Quaternion<double> rotation(qw, qx, qy, qz);
    Translation<double, 3> translation(tx, ty, tz);
    Affine3d transform = translation * rotation;
    traj.set(idx, transform);
  }

  traj.save(dst);
  cout << "Saved to " << dst << endl;
  frei.close();
  return 0;
}
