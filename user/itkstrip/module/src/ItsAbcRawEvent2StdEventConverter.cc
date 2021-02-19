#include "eudaq/StdEventConverter.hh"
#include "eudaq/RawEvent.hh"
#include <regex>
class ItsAbcRawEvent2StdEventConverter: public eudaq::StdEventConverter{
  // mode magic
  enum class Mode { DEFAULT, Nov20_3planes };
  std::map<std::string,Mode> modeDict { std::make_pair("Nov20_3planes",Mode::Nov20_3planes), std::make_pair("default",Mode::DEFAULT) };
  Mode getMode(eudaq::ConfigSPC conf) const {
    std::string modeStr = conf->Get("mode","default");
    auto it = modeDict.find(modeStr);
    if(it == modeDict.end()){
      EUDAQ_THROW("unknown mode error: "+modeStr);
    }
    return it->second;
  }
  // blockmap stuff
public:
  typedef std::map<uint32_t, std::pair<uint32_t, uint32_t> > BlockMap;
private:
  inline BlockMap initializeBlockMap(Mode mode) const {
    switch(mode){
    case Mode::DEFAULT:
      return {{0,{0,1}},
              {1,{1,1}},
              {2,{0,0}},
              {3,{1,0}},
              {4,{0,2}},
              {5,{1,2}},
              {6,{2,1}},
              {7,{3,1}},
              {8,{2,0}},
              {9,{3,0}},
              {10,{2,2}},
              {11,{3,2}},
              {12,{0,4}},
              {13,{1,4}},
              {14,{2,4}},
              {15,{3,4}}};
    case Mode::Nov20_3planes:
      return {{0,{0,1}},
              {1,{1,1}},
              {2,{0,4}},
              {3,{1,4}}};
    default:
      EUDAQ_THROW("unknown mode requested for initializeBlockMap");
    }
  }
  
public:
  bool Converting(eudaq::EventSPC d1, eudaq::StdEventSP d2, eudaq::ConfigSPC conf) const override;
  static const uint32_t m_id_factory = eudaq::cstr2hash("ITS_ABC");
  static const uint32_t PLANE_ID_OFFSET_ABC = 10;
};
namespace{
  auto dummy0 = eudaq::Factory<eudaq::StdEventConverter>::
    Register<ItsAbcRawEvent2StdEventConverter>(ItsAbcRawEvent2StdEventConverter::m_id_factory);
}
// Normally, will be exactly the same every event, so remember
bool ItsAbcRawEvent2StdEventConverter::Converting(eudaq::EventSPC d1, eudaq::StdEventSP d2,
                                                  eudaq::ConfigSPC conf) const{
  auto raw = std::dynamic_pointer_cast<const eudaq::RawEvent>(d1);
  auto mode = getMode(conf);
  if (!raw){
    EUDAQ_THROW("dynamic_cast error: from eudaq::Event to eudaq::RawEvent");
  }  
  std::string block_dsp = raw->GetTag("ABC_EVENT", "");
  if(block_dsp.empty()){
    return true;
  }
  BlockMap block_map = initializeBlockMap(mode);
    
  if(mode == Mode::Nov20_3planes && block_dsp.compare("(0:0:1,1:1:1,2:0:r,3:1:r)") != 0){
    EUDAQ_THROW("block map needs updating: "+block_dsp);
  }

  auto block_n_list = raw->GetBlockNumList();
  for(auto &block_n: block_n_list){
    if(mode == Mode::Nov20_3planes && block_n != 0)
      continue;
   
    auto it = block_map.find(block_n);

    if(it == block_map.end())
      continue;

    if(it->second.second<3){
      //raw data first stream
      std::vector<uint8_t> block = raw->GetBlock(block_n);
      std::vector<bool> channels;
      eudaq::uchar2bool(block.data(), block.data() + block.size(), channels);
      if(mode == Mode::DEFAULT){
        uint32_t strN = it->second.first;
        uint32_t bcN = it->second.second;
        uint32_t plane_id = PLANE_ID_OFFSET_ABC + bcN*10 + strN;

        eudaq::StandardPlane plane(plane_id, "ITS_ABC", "ABC");
        // plane.SetSizeZS(channels.size(), 1, 0);//r0
        plane.SetSizeZS(1,channels.size(), 0);//ss
        for(size_t i = 0; i < channels.size(); ++i) {
          if(channels[i]){
            // plane.PushPixel(i, 1 , 1);//r0
            plane.PushPixel(1, i , 1);//ss
          }
        }
        d2->AddPlane(plane);
        
      } else if(mode == Mode::Nov20_3planes){
        uint32_t plane_id = 8;

        //raw data second stream
        std::vector<uint8_t> my_block = raw->GetBlock(block_n + 1);
        std::vector<bool> my_channels;
        eudaq::uchar2bool(my_block.data(), my_block.data() + my_block.size(), my_channels);
        
        // MINI 0 plane
        eudaq::StandardPlane plane_0(10, "ITS_ABC", "ABC");
        d2->SetDetectorType("ITk_strip");
        //plane_0.SetSizeZS(channels.size(), 1, 0);//r0
        plane_0.SetSizeZS(105, 1, 0);//r0
        
        // MINI 1 plane
        eudaq::StandardPlane plane_1(12, "ITS_ABC", "ABC");
        d2->SetDetectorType("ITk_strip");
        //plane_1.SetSizeZS(channels.size(), 1, 0);//r0
        plane_1.SetSizeZS(105, 1, 0);//r0
        
        // MINI 2 plane
        eudaq::StandardPlane plane_2(11, "ITS_ABC", "ABC");
        d2->SetDetectorType("ITk_strip");
        //plane_2.SetSizeZS(my_channels.size(), 1, 0);//r0
        plane_2.SetSizeZS(105, 1, 0);//r0
        
        // plane.SetSizeZS(1,channels.size(), 0);//ss
        for(size_t i = 0; i < channels.size(); ++i) {
          if(i < 530){
            if(channels[i])
              if(i > 480 && i % 2 != 0)
                //plane_0.PushPixel(i, 0 , 1);
                plane_0.PushPixel(i-408, 0 , 1);
              else
                plane_0.PushPixel(i-406, 0 , 1);
          }
        }
        
        d2->AddPlane(plane_0);
        
        for(size_t i = 0; i < channels.size(); i++){
          if(i > 530){
            if(channels[i]){
              //plane_1.PushPixel(i, 1 , 1);
              plane_1.PushPixel(i-577, 0 , 1);
            }
          }
        }
        
        d2->AddPlane(plane_1);
        
        for(size_t i = 0; i < my_channels.size(); i++){
          if(my_channels[i]){
            //plane_2.PushPixel(i, 1 , 1);
            plane_2.PushPixel(i-513, 0 , 1);
          }
        }
        
        d2->AddPlane(plane_2);
      } 
    }
  }
  return true;
}
