#include <iostream>
#include <vector>
#include <unordered_map>
#include <google/protobuf/util/json_util.h>
#include "proto/model_feature.pb.h"
#include "proto/tf_feature.pb.h"
//#include "model_feature.pb.h"


struct FeatureResult {
    // key: feature_name, value: val
    std::map<std::string, float> float_features;
    // key: feature_name, value: sparse_id
    std::map<std::string, long> int_features;
    // key: feature_name, value: sequence_id
    std::map<std::string, std::vector<long>> sequence_features;
    void merge(std::shared_ptr<FeatureResult> ptr);
};


typedef std::shared_ptr<FeatureResult> FeatureResultPtr;

int hash_feature(const std::string& name, int32_t id);

class ModelFeature {
public:
    ModelFeature() {
        feature_result = std::make_shared<FeatureResult>();
    }

    std::string extract_json(const std::string& str);
	std::string extract_tf_example(const std::string& str);
    FeatureResultPtr extract_feature(const Feature& feature);
    FeatureResultPtr extract_user_feature(const UserProfile& up);
    FeatureResultPtr extract_ad_feature(const AdData& ad);
    FeatureResultPtr extract_user_ad_feature(const UserAdFeature& uaf);
    FeatureResultPtr extract_ctx_feature(const Context& ctx);

    void print() {
        for (auto& it : feature_result->int_features) {
            std::cout<<it.first<<" "<<it.second<<"\n";
        }
    }
private:
    void extract_user_profile(const std::string& prefix, const UserProfile& up);
    void extract_ad_data(const std::string& prefix, const AdData& ad);
    void extract_user_ad_feature(const std::string& prefix, const UserAdFeature& uaf);
    void extract_context(const std::string& prefix, const Context& ctx);

    void extract_user_base(const std::string& prefix, const UserBase& ub);
    void extract_user_behavior(const std::string& prefix, const UserBehavior& ub);
    void extract_user_count(const std::string& prefix, const UserCount& uc);

    void extract_bcf(const std::string& prefix, const BusinessCountFeature& bcf);
    void extract_acf(const std::string& prefix, const BusinessCountFeature_ActionCountFeature& acf);

    void extract_ad_info(const std::string& prefix, const AdInfo& ai);
    void extract_ad_count(const std::string& prefix, const AdCount& ac);
    void extract_creative(const std::string& prefix, const Creative& c, bool is_seq);

    void extract_user_ad_count(const std::string& prefix, const UserAdCount& uac);


    void append(const std::string& prefix, const std::string& name, int32_t fid, bool is_seq=false) {
        std::string key = prefix + name;
        /*
        auto it = feature_space.find(key);
        if (it == feature_space.end()) {
            return;
        }
        auto space = it->second;
        */
		fid = hash_feature(key, fid);
        if (is_seq) {
            feature_result->sequence_features[key].push_back(fid);
        } else {
            feature_result->int_features[key] = fid;
        }
    }
    FeatureResultPtr feature_result;
};
