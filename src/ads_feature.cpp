#include <cmath>
#include <sstream>
#include <functional>
#include <google/protobuf/util/json_util.h>
#include <cstdint>
#include "ads_feature.h"

void FeatureResult::merge(std::shared_ptr<FeatureResult> ptr) {
    for (auto it : ptr->int_features) {
        int_features[it.first] = it.second;
    }
    for (auto it : ptr->sequence_features) {
        sequence_features[it.first] = it.second;
    }
    for (auto it : ptr->float_features) {
        float_features[it.first] = it.second;
    }
}

inline uint32_t time33(const char* str, size_t key_length) {
    uint32_t hash = 5381;
    while (key_length--) {
        hash += (hash << 5) + (*str++);
    }
    return (hash & 0x7FFFFFFF);
}

template<typename T>
int32_t hash(const T t) {
    return time33((const char*)&t, sizeof(T));
}

template<typename T, typename H>
int32_t hash(const T& t, const H& h) {
    uint32_t id = hash(std::to_string(t) + std::to_string(h));
    return id;
}

template<typename T>
int32_t log(T t) {
    return int32_t(std::log(std::max(t, (T)1))*100);
}

template<typename T, typename H>
double log_div(T a, H b) {
    if (b == 0) {
        return log(a);
    } else {
        return log(double(a) / b);
    }
}

std::string ModelFeature::extract_json(const std::string& str) {
    Sample sample;
    auto status = google::protobuf::util::JsonStringToMessage(str, &sample);
    if (status.ok()) {
        auto result = extract_feature(sample.feature());
        std::ostringstream oss;
        oss << "{\"label\": {"
            << "\"imp\":" <<sample.label().imp()
            << ",\"click\":" <<sample.label().click()
            << ",\"install\":" <<sample.label().install()
            << ",\"attr_install\":" <<sample.label().attr_install()
            <<"},\"int_features\": {";
        int index = 0;
        for (auto it : result->int_features) {
            if (index++ > 0) {
                oss <<",";
            }
            oss << "\"" << it.first <<"\": " << it.second;
        }

        oss << "},\"float_features\": {";
        index = 0;
        for (auto it : result->float_features) {
            if (index++ > 0) {
                oss <<",";
            }
            oss << "\"" << it.first <<"\": " << it.second;
        }

        oss<<"}, \"sequence_features\": {";
        index = 0;
        for (auto it : result->sequence_features) {
            if (index++ > 0) {
                oss <<",";
            }
            oss << "\"" << it.first <<"\": [";
            int _index = 0;
            for (auto x : it.second) {
                if (_index ++ > 0) {
                    oss<<",";
                }
                oss<<x;
            }
            oss<<"]";
        }
        oss<<"}}";
        return oss.str();
    } else {
        std::cout<<"error json\n";
        return "";
    }
}

FeatureResultPtr ModelFeature::extract_feature(const Feature& feature) {
    extract_user_feature(feature.user_profile());
    extract_ad_feature(feature.ad_data());
    extract_user_ad_feature(feature.user_ad_feature());
    extract_ctx_feature(feature.context());
    return feature_result;
}

FeatureResultPtr ModelFeature::extract_user_feature(const UserProfile& up) {
    extract_user_profile("up", up);
    return feature_result;
}
FeatureResultPtr ModelFeature::extract_ad_feature(const AdData& ad) {
    extract_ad_data("user_ad", ad);
    return feature_result;
}
FeatureResultPtr ModelFeature::extract_user_ad_feature(const UserAdFeature& uaf) {
    extract_user_ad_feature("user_ad", uaf);
    return feature_result;
}
FeatureResultPtr ModelFeature::extract_ctx_feature(const Context& ctx) {
    extract_context("ctx", ctx);
    return feature_result;
}


void ModelFeature::extract_acf(const std::string& prefix, const BusinessCountFeature_ActionCountFeature& acf) {
    auto imp = log(acf.imp());
    auto click = log(acf.click());
    auto install = log(acf.install());
    auto attr_install = log(acf.attr_install());

    auto imp_click = log_div(acf.imp(), acf.click());
    auto imp_install = log_div(acf.imp(), acf.install());
    auto imp_attr_install = log_div(acf.imp(), acf.attr_install());
    auto click_install = log_div(acf.click(), acf.install());
    auto click_attr_install = log_div(acf.click(), acf.attr_install());
    auto install_attr_install = log_div(acf.install(), acf.attr_install());

    append(prefix, "_imp", hash(imp));
    append(prefix, "_clk", hash(click));
    append(prefix, "_inst", hash(install));
    append(prefix, "_ainst", hash(attr_install));

    append(prefix, "_imp_clk", hash(imp, click));
    append(prefix, "_imp_inst", hash(imp, install));
    append(prefix, "_imp_ainst", hash(imp, attr_install));
    append(prefix, "_clk_inst", hash(click, install));
    append(prefix, "_clk_ainst", hash(click, attr_install));
    append(prefix, "_inst_ainst", hash(install, attr_install));

    append(prefix, "_imp/clk", hash(imp, imp_click));
    append(prefix, "_imp/inst", hash(imp, imp_install));
    append(prefix, "_imp/ainst", hash(imp, imp_attr_install));
    append(prefix, "_clk/inst", hash(click, click_install));
    append(prefix, "_clk/ainst", hash(click, click_attr_install));
    append(prefix, "_inst/ainst", hash(install, install_attr_install));
}

void ModelFeature::extract_bcf(const std::string& prefix, const BusinessCountFeature& bcf) {
    if (bcf.has_count_features_7d()) {
        extract_acf(prefix + "_7d", bcf.count_features_7d());
    }
    if (bcf.has_count_features_14d()) {
        extract_acf(prefix + "_14d", bcf.count_features_14d());
    }
    if (bcf.has_count_features_30d()) {
        extract_acf(prefix + "_30d", bcf.count_features_30d());
    }
}

void ModelFeature::extract_creative(const std::string& prefix, const Creative& c, bool is_seq) {
    //if (c.has_creative_id()) {
        append(prefix, "_cid", hash(c.creative_id()), is_seq);
    //}
    //if (c.has_cp_id()) {
        append(prefix, "_cpid", hash(c.cp_id()), is_seq);
    //}
}

void ModelFeature::extract_ad_info(const std::string& prefix, const AdInfo& ai) {
    //if (ai.has_ad_id()) {
        append(prefix, "_adid", hash(std::to_string(ai.ad_id())));
    //}
    //if (ai.has_app_id()) {
        append(prefix, "_aid", hash(ai.app_id()));
    //}
    //if (ai.has_category()) {
        append(prefix, "_cate", hash(ai.category()));
    //}
    for (auto & c : ai.creatives()) {
        extract_creative(prefix + "_c_", c, true);
    }

    //if (ai.has_attr_platform()) {
        append(prefix, "_attp", hash(std::to_string(ai.attr_platform())));
    //}
    //if (ai.has_is_auto_download()) {
        append(prefix, "_attd", hash(std::to_string(ai.is_auto_download())));
    //}
}

void ModelFeature::extract_ad_data(const std::string& prefix, const AdData& ad) {
    //if (ad.has_ad_info()) {
        extract_ad_info(prefix + "_ai", ad.ad_info());
    //}
    //if (ad.has_ad_counter()) {
        extract_ad_count(prefix + "_ac", ad.ad_counter());
    //}
}

void ModelFeature::extract_context(const std::string& prefix, const Context& ctx) {
    //if (ctx.has_pos_id()) {
        append(prefix, "_pi1", hash(ctx.pos_id()));
    //}
    //if (ctx.has_app_name()) {
        append(prefix, "_app2", hash(ctx.app_name()));
    //}
    //if (ctx.has_network_type()) {
        append(prefix, "_ne3", hash(ctx.network_type()));
    //}
    //if (ctx.has_os_version()) {
        append(prefix, "_os4", hash(ctx.os_version()));
    //}
    //if (ctx.has_brand()) {
        append(prefix, "_brand", hash(ctx.brand()));
    //}
    //if (ctx.has_model()) {
        append(prefix, "_model", hash(ctx.model()));
    //}
    //if (ctx.has_app_version_code()) {
        append(prefix, "_app_ver", hash(ctx.app_version_code()));
    //}
    auto ip = ctx.client_ip();
    // ToDo: 拆ip段
    append(prefix, "_ip", hash(ctx.client_ip()));
    //if (ctx.has_nation()) {
        append(prefix, "_nation", hash(ctx.nation()));
    //}
    //if (ctx.has_req_time()) {
        auto time = ctx.req_time();
        auto hour = time / 1000 % 3600;
        auto week = time / 1000 / 86400 % 7;
        append(prefix, "_req_time", hash(time));
        append(prefix, "_req_hour", hash(hour));
        append(prefix, "_req_week", hash(week));
    //}
}

void ModelFeature::extract_user_ad_count(const std::string& prefix, const UserAdCount& uac) {
    if (uac.has_user_id_is_auto_download()) {
        extract_bcf(prefix + "_bcf1", uac.user_id_is_auto_download());
    }
    if (uac.has_user_id_ad_id_is_auto_download()) {
        extract_bcf(prefix + "_bcf2", uac.user_id_ad_id_is_auto_download());
    }
    if (uac.has_user_id_ad_package_name_is_auto_download()) {
        extract_bcf(prefix + "_bcf3", uac.user_id_ad_package_name_is_auto_download());
    }
    if (uac.has_user_id_ad_package_category_is_auto_download()) {
        extract_bcf(prefix + "_bcf4", uac.user_id_ad_package_category_is_auto_download());
    }
    if (uac.has_user_id_pos_id_ad_id_is_auto_download()) {
        extract_bcf(prefix + "_bcf5", uac.user_id_pos_id_ad_id_is_auto_download());
    }
    if (uac.has_user_id_pos_id_ad_package_name_is_auto_download()) {
        extract_bcf(prefix + "_bcf6", uac.user_id_pos_id_ad_package_name_is_auto_download());
    }
}

void ModelFeature::extract_user_ad_feature(const std::string& prefix, const UserAdFeature& uaf) {
    if (uaf.has_user_ad_count()) {
        extract_user_ad_count(prefix + "_uac", uaf.user_ad_count());
    }
}

void ModelFeature::extract_ad_count(const std::string& prefix, const AdCount& ac) {
    if (ac.has_ad_id()) {
        extract_bcf(prefix + "_bcf1", ac.ad_id());
    }
    if (ac.has_ad_package_name()) {
        extract_bcf(prefix + "_bcf2", ac.ad_package_name());
    }
    if (ac.has_ad_package_category()) {
        extract_bcf(prefix + "_bcf3", ac.ad_package_category());
    }
    if (ac.has_pos_id_ad_id()) {
        extract_bcf(prefix + "_bcf4", ac.pos_id_ad_id());
    }
    if (ac.has_pos_id_ad_package_name()) {
        extract_bcf(prefix + "_bcf5", ac.pos_id_ad_package_name());
    }
    if (ac.has_is_auto_download()) {
        extract_bcf(prefix + "_bcf6", ac.is_auto_download());
    }
}

void ModelFeature::extract_user_count(const std::string& prefix, const  UserCount& uc) {
    if (uc.has_user_id()) {
        extract_bcf(prefix + "_uid", uc.user_id());
    }
}

void ModelFeature::extract_user_behavior(const std::string& prefix, const  UserBehavior& ub) {
    for (auto & app : ub.click_ad_app_list()) {
        append(prefix, "_caal", hash(app), true);
    }
    for (auto & app : ub.install_ad_app_list()) {
        append(prefix, "_iaal", hash(app), true);
    }
    for (auto & app : ub.install_app_list()) {
        append(prefix, "_ial", hash(app), true);
    }
    for (auto & app : ub.use_app_list()) {
        append(prefix, "_ual", hash(app), true);
    }
}

void ModelFeature::extract_user_base(const std::string& prefix, const UserBase& ub) {
    //if (ub.has_country()) {
        append(prefix, "_country", hash(ub.country()));
    //}
    //if (ub.has_province()) {
        append(prefix, "_province", hash(ub.province()));
    //}
    //if (ub.has_city()) {
        append(prefix, "_city", hash(ub.city()));
    //}
    //if (ub.has_gender()) {
        append(prefix, "_gender", hash(ub.gender()));
    //}
    //if (ub.has_age()) {
        append(prefix, "_age", hash(ub.age()));
    //}
}

void ModelFeature::extract_user_profile(const std::string& prefix, const UserProfile& up) {
    append(prefix, "_uid", hash(up.user_id()));
    if (up.has_user_base()) {
        extract_user_base(prefix + "_ub2", up.user_base());
    }
    if (up.has_user_behavior()) {
        extract_user_behavior(prefix + "_ub3", up.user_behavior());
    }
    if (up.has_user_counter()) {
        extract_user_count(prefix + "_uc", up.user_counter());
    }
}
