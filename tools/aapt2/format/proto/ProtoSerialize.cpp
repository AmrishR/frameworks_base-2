/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "format/proto/ProtoSerialize.h"

#include "ValueVisitor.h"
#include "util/BigBuffer.h"

namespace aapt {

void SerializeStringPoolToPb(const StringPool& pool, pb::StringPool* out_pb_pool, IDiagnostics* diag) {
  BigBuffer buffer(1024);
  StringPool::FlattenUtf8(&buffer, pool, diag);

  std::string* data = out_pb_pool->mutable_data();
  data->reserve(buffer.size());

  size_t offset = 0;
  for (const BigBuffer::Block& block : buffer) {
    data->insert(data->begin() + offset, block.buffer.get(), block.buffer.get() + block.size);
    offset += block.size;
  }
}

void SerializeSourceToPb(const Source& source, StringPool* src_pool, pb::Source* out_pb_source) {
  StringPool::Ref ref = src_pool->MakeRef(source.path);
  out_pb_source->set_path_idx(static_cast<uint32_t>(ref.index()));
  if (source.line) {
    out_pb_source->mutable_position()->set_line_number(static_cast<uint32_t>(source.line.value()));
  }
}

static pb::Visibility::Level SerializeVisibilityToPb(Visibility::Level state) {
  switch (state) {
    case Visibility::Level::kPrivate:
      return pb::Visibility::PRIVATE;
    case Visibility::Level::kPublic:
      return pb::Visibility::PUBLIC;
    default:
      break;
  }
  return pb::Visibility::UNKNOWN;
}

void SerializeConfig(const ConfigDescription& config, pb::Configuration* out_pb_config) {
  out_pb_config->set_mcc(config.mcc);
  out_pb_config->set_mnc(config.mnc);
  out_pb_config->set_locale(config.GetBcp47LanguageTag());

  switch (config.screenLayout & ConfigDescription::MASK_LAYOUTDIR) {
    case ConfigDescription::LAYOUTDIR_LTR:
      out_pb_config->set_layout_direction(pb::Configuration_LayoutDirection_LAYOUT_DIRECTION_LTR);
      break;

    case ConfigDescription::LAYOUTDIR_RTL:
      out_pb_config->set_layout_direction(pb::Configuration_LayoutDirection_LAYOUT_DIRECTION_RTL);
      break;
  }

  out_pb_config->set_screen_width(config.screenWidth);
  out_pb_config->set_screen_height(config.screenHeight);
  out_pb_config->set_screen_width_dp(config.screenWidthDp);
  out_pb_config->set_screen_height_dp(config.screenHeightDp);
  out_pb_config->set_smallest_screen_width_dp(config.smallestScreenWidthDp);

  switch (config.screenLayout & ConfigDescription::MASK_SCREENSIZE) {
    case ConfigDescription::SCREENSIZE_SMALL:
      out_pb_config->set_screen_layout_size(
          pb::Configuration_ScreenLayoutSize_SCREEN_LAYOUT_SIZE_SMALL);
      break;

    case ConfigDescription::SCREENSIZE_NORMAL:
      out_pb_config->set_screen_layout_size(
          pb::Configuration_ScreenLayoutSize_SCREEN_LAYOUT_SIZE_NORMAL);
      break;

    case ConfigDescription::SCREENSIZE_LARGE:
      out_pb_config->set_screen_layout_size(
          pb::Configuration_ScreenLayoutSize_SCREEN_LAYOUT_SIZE_LARGE);
      break;

    case ConfigDescription::SCREENSIZE_XLARGE:
      out_pb_config->set_screen_layout_size(
          pb::Configuration_ScreenLayoutSize_SCREEN_LAYOUT_SIZE_XLARGE);
      break;
  }

  switch (config.screenLayout & ConfigDescription::MASK_SCREENLONG) {
    case ConfigDescription::SCREENLONG_YES:
      out_pb_config->set_screen_layout_long(
          pb::Configuration_ScreenLayoutLong_SCREEN_LAYOUT_LONG_LONG);
      break;

    case ConfigDescription::SCREENLONG_NO:
      out_pb_config->set_screen_layout_long(
          pb::Configuration_ScreenLayoutLong_SCREEN_LAYOUT_LONG_NOTLONG);
      break;
  }

  switch (config.screenLayout2 & ConfigDescription::MASK_SCREENROUND) {
    case ConfigDescription::SCREENROUND_YES:
      out_pb_config->set_screen_round(pb::Configuration_ScreenRound_SCREEN_ROUND_ROUND);
      break;

    case ConfigDescription::SCREENROUND_NO:
      out_pb_config->set_screen_round(pb::Configuration_ScreenRound_SCREEN_ROUND_NOTROUND);
      break;
  }

  switch (config.colorMode & ConfigDescription::MASK_WIDE_COLOR_GAMUT) {
    case ConfigDescription::WIDE_COLOR_GAMUT_YES:
      out_pb_config->set_wide_color_gamut(pb::Configuration_WideColorGamut_WIDE_COLOR_GAMUT_WIDECG);
      break;

    case ConfigDescription::WIDE_COLOR_GAMUT_NO:
      out_pb_config->set_wide_color_gamut(
          pb::Configuration_WideColorGamut_WIDE_COLOR_GAMUT_NOWIDECG);
      break;
  }

  switch (config.colorMode & ConfigDescription::MASK_HDR) {
    case ConfigDescription::HDR_YES:
      out_pb_config->set_hdr(pb::Configuration_Hdr_HDR_HIGHDR);
      break;

    case ConfigDescription::HDR_NO:
      out_pb_config->set_hdr(pb::Configuration_Hdr_HDR_LOWDR);
      break;
  }

  switch (config.orientation) {
    case ConfigDescription::ORIENTATION_PORT:
      out_pb_config->set_orientation(pb::Configuration_Orientation_ORIENTATION_PORT);
      break;

    case ConfigDescription::ORIENTATION_LAND:
      out_pb_config->set_orientation(pb::Configuration_Orientation_ORIENTATION_LAND);
      break;

    case ConfigDescription::ORIENTATION_SQUARE:
      out_pb_config->set_orientation(pb::Configuration_Orientation_ORIENTATION_SQUARE);
      break;
  }

  switch (config.uiMode & ConfigDescription::MASK_UI_MODE_TYPE) {
    case ConfigDescription::UI_MODE_TYPE_NORMAL:
      out_pb_config->set_ui_mode_type(pb::Configuration_UiModeType_UI_MODE_TYPE_NORMAL);
      break;

    case ConfigDescription::UI_MODE_TYPE_DESK:
      out_pb_config->set_ui_mode_type(pb::Configuration_UiModeType_UI_MODE_TYPE_DESK);
      break;

    case ConfigDescription::UI_MODE_TYPE_CAR:
      out_pb_config->set_ui_mode_type(pb::Configuration_UiModeType_UI_MODE_TYPE_CAR);
      break;

    case ConfigDescription::UI_MODE_TYPE_TELEVISION:
      out_pb_config->set_ui_mode_type(pb::Configuration_UiModeType_UI_MODE_TYPE_TELEVISION);
      break;

    case ConfigDescription::UI_MODE_TYPE_APPLIANCE:
      out_pb_config->set_ui_mode_type(pb::Configuration_UiModeType_UI_MODE_TYPE_APPLIANCE);
      break;

    case ConfigDescription::UI_MODE_TYPE_WATCH:
      out_pb_config->set_ui_mode_type(pb::Configuration_UiModeType_UI_MODE_TYPE_WATCH);
      break;

    case ConfigDescription::UI_MODE_TYPE_VR_HEADSET:
      out_pb_config->set_ui_mode_type(pb::Configuration_UiModeType_UI_MODE_TYPE_VRHEADSET);
      break;
  }

  switch (config.uiMode & ConfigDescription::MASK_UI_MODE_NIGHT) {
    case ConfigDescription::UI_MODE_NIGHT_YES:
      out_pb_config->set_ui_mode_night(pb::Configuration_UiModeNight_UI_MODE_NIGHT_NIGHT);
      break;

    case ConfigDescription::UI_MODE_NIGHT_NO:
      out_pb_config->set_ui_mode_night(pb::Configuration_UiModeNight_UI_MODE_NIGHT_NOTNIGHT);
      break;
  }

  out_pb_config->set_density(config.density);

  switch (config.touchscreen) {
    case ConfigDescription::TOUCHSCREEN_NOTOUCH:
      out_pb_config->set_touchscreen(pb::Configuration_Touchscreen_TOUCHSCREEN_NOTOUCH);
      break;

    case ConfigDescription::TOUCHSCREEN_STYLUS:
      out_pb_config->set_touchscreen(pb::Configuration_Touchscreen_TOUCHSCREEN_STYLUS);
      break;

    case ConfigDescription::TOUCHSCREEN_FINGER:
      out_pb_config->set_touchscreen(pb::Configuration_Touchscreen_TOUCHSCREEN_FINGER);
      break;
  }

  switch (config.inputFlags & ConfigDescription::MASK_KEYSHIDDEN) {
    case ConfigDescription::KEYSHIDDEN_NO:
      out_pb_config->set_keys_hidden(pb::Configuration_KeysHidden_KEYS_HIDDEN_KEYSEXPOSED);
      break;

    case ConfigDescription::KEYSHIDDEN_YES:
      out_pb_config->set_keys_hidden(pb::Configuration_KeysHidden_KEYS_HIDDEN_KEYSHIDDEN);
      break;

    case ConfigDescription::KEYSHIDDEN_SOFT:
      out_pb_config->set_keys_hidden(pb::Configuration_KeysHidden_KEYS_HIDDEN_KEYSSOFT);
      break;
  }

  switch (config.keyboard) {
    case ConfigDescription::KEYBOARD_NOKEYS:
      out_pb_config->set_keyboard(pb::Configuration_Keyboard_KEYBOARD_NOKEYS);
      break;

    case ConfigDescription::KEYBOARD_QWERTY:
      out_pb_config->set_keyboard(pb::Configuration_Keyboard_KEYBOARD_QWERTY);
      break;

    case ConfigDescription::KEYBOARD_12KEY:
      out_pb_config->set_keyboard(pb::Configuration_Keyboard_KEYBOARD_TWELVEKEY);
      break;
  }

  switch (config.inputFlags & ConfigDescription::MASK_NAVHIDDEN) {
    case ConfigDescription::NAVHIDDEN_NO:
      out_pb_config->set_nav_hidden(pb::Configuration_NavHidden_NAV_HIDDEN_NAVEXPOSED);
      break;

    case ConfigDescription::NAVHIDDEN_YES:
      out_pb_config->set_nav_hidden(pb::Configuration_NavHidden_NAV_HIDDEN_NAVHIDDEN);
      break;
  }

  switch (config.navigation) {
    case ConfigDescription::NAVIGATION_NONAV:
      out_pb_config->set_navigation(pb::Configuration_Navigation_NAVIGATION_NONAV);
      break;

    case ConfigDescription::NAVIGATION_DPAD:
      out_pb_config->set_navigation(pb::Configuration_Navigation_NAVIGATION_DPAD);
      break;

    case ConfigDescription::NAVIGATION_TRACKBALL:
      out_pb_config->set_navigation(pb::Configuration_Navigation_NAVIGATION_TRACKBALL);
      break;

    case ConfigDescription::NAVIGATION_WHEEL:
      out_pb_config->set_navigation(pb::Configuration_Navigation_NAVIGATION_WHEEL);
      break;
  }

  out_pb_config->set_sdk_version(config.sdkVersion);
}

void SerializeTableToPb(const ResourceTable& table, pb::ResourceTable* out_table,
                        IDiagnostics* diag) {
  StringPool source_pool;
  for (const std::unique_ptr<ResourceTablePackage>& package : table.packages) {
    pb::Package* pb_package = out_table->add_package();
    if (package->id) {
      pb_package->mutable_package_id()->set_id(package->id.value());
    }
    pb_package->set_package_name(package->name);

    for (const std::unique_ptr<ResourceTableType>& type : package->types) {
      pb::Type* pb_type = pb_package->add_type();
      if (type->id) {
        pb_type->mutable_type_id()->set_id(type->id.value());
      }
      pb_type->set_name(to_string(type->type).to_string());

      for (const std::unique_ptr<ResourceEntry>& entry : type->entries) {
        pb::Entry* pb_entry = pb_type->add_entry();
        if (entry->id) {
          pb_entry->mutable_entry_id()->set_id(entry->id.value());
        }
        pb_entry->set_name(entry->name);

        // Write the Visibility struct.
        pb::Visibility* pb_visibility = pb_entry->mutable_visibility();
        pb_visibility->set_level(SerializeVisibilityToPb(entry->visibility.level));
        SerializeSourceToPb(entry->visibility.source, &source_pool,
                            pb_visibility->mutable_source());
        pb_visibility->set_comment(entry->visibility.comment);

        if (entry->allow_new) {
          pb::AllowNew* pb_allow_new = pb_entry->mutable_allow_new();
          SerializeSourceToPb(entry->allow_new.value().source, &source_pool,
                              pb_allow_new->mutable_source());
          pb_allow_new->set_comment(entry->allow_new.value().comment);
        }

        if (entry->overlayable) {
          pb::Overlayable* pb_overlayable = pb_entry->mutable_overlayable();
          SerializeSourceToPb(entry->overlayable.value().source, &source_pool,
                              pb_overlayable->mutable_source());
          pb_overlayable->set_comment(entry->overlayable.value().comment);
        }

        for (const std::unique_ptr<ResourceConfigValue>& config_value : entry->values) {
          pb::ConfigValue* pb_config_value = pb_entry->add_config_value();
          SerializeConfig(config_value->config, pb_config_value->mutable_config());
          pb_config_value->mutable_config()->set_product(config_value->product);
          SerializeValueToPb(*config_value->value, pb_config_value->mutable_value(), &source_pool);
        }
      }
    }
  }
  SerializeStringPoolToPb(source_pool, out_table->mutable_source_pool(), diag);
}

static pb::Reference_Type SerializeReferenceTypeToPb(Reference::Type type) {
  switch (type) {
    case Reference::Type::kResource:
      return pb::Reference_Type_REFERENCE;
    case Reference::Type::kAttribute:
      return pb::Reference_Type_ATTRIBUTE;
    default:
      break;
  }
  return pb::Reference_Type_REFERENCE;
}

static void SerializeReferenceToPb(const Reference& ref, pb::Reference* pb_ref) {
  pb_ref->set_id(ref.id.value_or_default(ResourceId(0x0)).id);

  if (ref.name) {
    pb_ref->set_name(ref.name.value().to_string());
  }

  pb_ref->set_private_(ref.private_reference);
  pb_ref->set_type(SerializeReferenceTypeToPb(ref.reference_type));
}

template <typename T>
static void SerializeItemMetaDataToPb(const Item& item, T* pb_item, StringPool* src_pool) {
  if (src_pool != nullptr) {
    SerializeSourceToPb(item.GetSource(), src_pool, pb_item->mutable_source());
  }
  pb_item->set_comment(item.GetComment());
}

static pb::Plural_Arity SerializePluralEnumToPb(size_t plural_idx) {
  switch (plural_idx) {
    case Plural::Zero:
      return pb::Plural_Arity_ZERO;
    case Plural::One:
      return pb::Plural_Arity_ONE;
    case Plural::Two:
      return pb::Plural_Arity_TWO;
    case Plural::Few:
      return pb::Plural_Arity_FEW;
    case Plural::Many:
      return pb::Plural_Arity_MANY;
    default:
      break;
  }
  return pb::Plural_Arity_OTHER;
}

static pb::FileReference::Type SerializeFileReferenceTypeToPb(const ResourceFile::Type& type) {
  switch (type) {
    case ResourceFile::Type::kBinaryXml:
      return pb::FileReference::BINARY_XML;
    case ResourceFile::Type::kProtoXml:
      return pb::FileReference::PROTO_XML;
    case ResourceFile::Type::kPng:
      return pb::FileReference::PNG;
    default:
      return pb::FileReference::UNKNOWN;
  }
}

namespace {

class ValueSerializer : public ConstValueVisitor {
 public:
  using ConstValueVisitor::Visit;

  ValueSerializer(pb::Value* out_value, StringPool* src_pool)
      : out_value_(out_value), src_pool_(src_pool) {
  }

  void Visit(const Reference* ref) override {
    SerializeReferenceToPb(*ref, out_value_->mutable_item()->mutable_ref());
  }

  void Visit(const String* str) override {
    out_value_->mutable_item()->mutable_str()->set_value(*str->value);
  }

  void Visit(const RawString* str) override {
    out_value_->mutable_item()->mutable_raw_str()->set_value(*str->value);
  }

  void Visit(const StyledString* str) override {
    pb::StyledString* pb_str = out_value_->mutable_item()->mutable_styled_str();
    pb_str->set_value(str->value->value);
    for (const StringPool::Span& span : str->value->spans) {
      pb::StyledString::Span* pb_span = pb_str->add_span();
      pb_span->set_tag(*span.name);
      pb_span->set_first_char(span.first_char);
      pb_span->set_last_char(span.last_char);
    }
  }

  void Visit(const FileReference* file) override {
    pb::FileReference* pb_file = out_value_->mutable_item()->mutable_file();
    pb_file->set_path(*file->path);
    pb_file->set_type(SerializeFileReferenceTypeToPb(file->type));
  }

  void Visit(const Id* /*id*/) override {
    out_value_->mutable_item()->mutable_id();
  }

  void Visit(const BinaryPrimitive* prim) override {
    android::Res_value val = {};
    prim->Flatten(&val);

    pb::Primitive* pb_prim = out_value_->mutable_item()->mutable_prim();

    switch (val.dataType) {
      case android::Res_value::TYPE_NULL: {
        if (val.data == android::Res_value::DATA_NULL_UNDEFINED) {
          pb_prim->set_allocated_null_value(new pb::Primitive_NullType());
        } else if (val.data == android::Res_value::DATA_NULL_EMPTY) {
          pb_prim->set_allocated_empty_value(new pb::Primitive_EmptyType());
        } else {
          LOG(FATAL) << "Unexpected data value for TYPE_NULL BinaryPrimitive: " << val.data;
        }
      } break;
      case android::Res_value::TYPE_FLOAT: {
        pb_prim->set_float_value(*(float*)&val.data);
      } break;
      case android::Res_value::TYPE_DIMENSION: {
        pb_prim->set_dimension_value(val.data);
      } break;
      case android::Res_value::TYPE_FRACTION: {
        pb_prim->set_fraction_value(val.data);
      } break;
      case android::Res_value::TYPE_INT_DEC: {
        pb_prim->set_int_decimal_value(static_cast<int32_t>(val.data));
      } break;
      case android::Res_value::TYPE_INT_HEX: {
        pb_prim->set_int_hexadecimal_value(val.data);
      } break;
      case android::Res_value::TYPE_INT_BOOLEAN: {
        pb_prim->set_boolean_value(static_cast<bool>(val.data));
      } break;
      case android::Res_value::TYPE_INT_COLOR_ARGB8: {
        pb_prim->set_color_argb8_value(val.data);
      } break;
      case android::Res_value::TYPE_INT_COLOR_RGB8: {
        pb_prim->set_color_rgb8_value(val.data);
      } break;
      case android::Res_value::TYPE_INT_COLOR_ARGB4: {
        pb_prim->set_color_argb4_value(val.data);
      } break;
      case android::Res_value::TYPE_INT_COLOR_RGB4: {
        pb_prim->set_color_rgb4_value(val.data);
      } break;
      default:
        LOG(FATAL) << "Unexpected BinaryPrimitive type: " << val.dataType;
        break;
    }
  }

  void Visit(const Attribute* attr) override {
    pb::Attribute* pb_attr = out_value_->mutable_compound_value()->mutable_attr();
    pb_attr->set_format_flags(attr->type_mask);
    pb_attr->set_min_int(attr->min_int);
    pb_attr->set_max_int(attr->max_int);

    for (auto& symbol : attr->symbols) {
      pb::Attribute_Symbol* pb_symbol = pb_attr->add_symbol();
      SerializeItemMetaDataToPb(symbol.symbol, pb_symbol, src_pool_);
      SerializeReferenceToPb(symbol.symbol, pb_symbol->mutable_name());
      pb_symbol->set_value(symbol.value);
    }
  }

  void Visit(const Style* style) override {
    pb::Style* pb_style = out_value_->mutable_compound_value()->mutable_style();
    if (style->parent) {
      const Reference& parent = style->parent.value();
      SerializeReferenceToPb(parent, pb_style->mutable_parent());
      if (src_pool_ != nullptr) {
        SerializeSourceToPb(parent.GetSource(), src_pool_, pb_style->mutable_parent_source());
      }
    }

    for (const Style::Entry& entry : style->entries) {
      pb::Style_Entry* pb_entry = pb_style->add_entry();
      SerializeReferenceToPb(entry.key, pb_entry->mutable_key());
      SerializeItemMetaDataToPb(entry.key, pb_entry, src_pool_);
      SerializeItemToPb(*entry.value, pb_entry->mutable_item());
    }
  }

  void Visit(const Styleable* styleable) override {
    pb::Styleable* pb_styleable = out_value_->mutable_compound_value()->mutable_styleable();
    for (const Reference& entry : styleable->entries) {
      pb::Styleable_Entry* pb_entry = pb_styleable->add_entry();
      SerializeItemMetaDataToPb(entry, pb_entry, src_pool_);
      SerializeReferenceToPb(entry, pb_entry->mutable_attr());
    }
  }

  void Visit(const Array* array) override {
    pb::Array* pb_array = out_value_->mutable_compound_value()->mutable_array();
    for (const std::unique_ptr<Item>& element : array->elements) {
      pb::Array_Element* pb_element = pb_array->add_element();
      SerializeItemMetaDataToPb(*element, pb_element, src_pool_);
      SerializeItemToPb(*element, pb_element->mutable_item());
    }
  }

  void Visit(const Plural* plural) override {
    pb::Plural* pb_plural = out_value_->mutable_compound_value()->mutable_plural();
    const size_t count = plural->values.size();
    for (size_t i = 0; i < count; i++) {
      if (!plural->values[i]) {
        // No plural value set here.
        continue;
      }

      pb::Plural_Entry* pb_entry = pb_plural->add_entry();
      pb_entry->set_arity(SerializePluralEnumToPb(i));
      SerializeItemMetaDataToPb(*plural->values[i], pb_entry, src_pool_);
      SerializeItemToPb(*plural->values[i], pb_entry->mutable_item());
    }
  }

  void VisitAny(const Value* unknown) override {
    LOG(FATAL) << "unimplemented value: " << *unknown;
  }

 private:
  pb::Value* out_value_;
  StringPool* src_pool_;
};

}  // namespace

void SerializeValueToPb(const Value& value, pb::Value* out_value, StringPool* src_pool) {
  ValueSerializer serializer(out_value, src_pool);
  value.Accept(&serializer);

  // Serialize the meta-data of the Value.
  out_value->set_comment(value.GetComment());
  out_value->set_weak(value.IsWeak());
  if (src_pool != nullptr) {
    SerializeSourceToPb(value.GetSource(), src_pool, out_value->mutable_source());
  }
}

void SerializeItemToPb(const Item& item, pb::Item* out_item) {
  pb::Value value;
  ValueSerializer serializer(&value, nullptr);
  item.Accept(&serializer);
  out_item->MergeFrom(value.item());
}

void SerializeCompiledFileToPb(const ResourceFile& file, pb::internal::CompiledFile* out_file) {
  out_file->set_resource_name(file.name.to_string());
  out_file->set_source_path(file.source.path);
  out_file->set_type(SerializeFileReferenceTypeToPb(file.type));
  SerializeConfig(file.config, out_file->mutable_config());

  for (const SourcedResourceName& exported : file.exported_symbols) {
    pb::internal::CompiledFile_Symbol* pb_symbol = out_file->add_exported_symbol();
    pb_symbol->set_resource_name(exported.name.to_string());
    pb_symbol->mutable_source()->set_line_number(exported.line);
  }
}

static void SerializeXmlCommon(const xml::Node& node, pb::XmlNode* out_node) {
  pb::SourcePosition* pb_src = out_node->mutable_source();
  pb_src->set_line_number(node.line_number);
  pb_src->set_column_number(node.column_number);
}

void SerializeXmlToPb(const xml::Element& el, pb::XmlNode* out_node) {
  SerializeXmlCommon(el, out_node);

  pb::XmlElement* pb_element = out_node->mutable_element();
  pb_element->set_name(el.name);
  pb_element->set_namespace_uri(el.namespace_uri);

  for (const xml::NamespaceDecl& ns : el.namespace_decls) {
    pb::XmlNamespace* pb_ns = pb_element->add_namespace_declaration();
    pb_ns->set_prefix(ns.prefix);
    pb_ns->set_uri(ns.uri);
    pb::SourcePosition* pb_src = pb_ns->mutable_source();
    pb_src->set_line_number(ns.line_number);
    pb_src->set_column_number(ns.column_number);
  }

  for (const xml::Attribute& attr : el.attributes) {
    pb::XmlAttribute* pb_attr = pb_element->add_attribute();
    pb_attr->set_name(attr.name);
    pb_attr->set_namespace_uri(attr.namespace_uri);
    pb_attr->set_value(attr.value);
    if (attr.compiled_attribute) {
      const ResourceId attr_id = attr.compiled_attribute.value().id.value_or_default({});
      pb_attr->set_resource_id(attr_id.id);
    }
    if (attr.compiled_value != nullptr) {
      SerializeItemToPb(*attr.compiled_value, pb_attr->mutable_compiled_item());
      pb::SourcePosition* pb_src = pb_attr->mutable_source();
      pb_src->set_line_number(attr.compiled_value->GetSource().line.value_or_default(0));
    }
  }

  for (const std::unique_ptr<xml::Node>& child : el.children) {
    if (const xml::Element* child_el = xml::NodeCast<xml::Element>(child.get())) {
      SerializeXmlToPb(*child_el, pb_element->add_child());
    } else if (const xml::Text* text_el = xml::NodeCast<xml::Text>(child.get())) {
      pb::XmlNode* pb_child_node = pb_element->add_child();
      SerializeXmlCommon(*text_el, pb_child_node);
      pb_child_node->set_text(text_el->text);
    } else {
      LOG(FATAL) << "unhandled XmlNode type";
    }
  }
}

void SerializeXmlResourceToPb(const xml::XmlResource& resource, pb::XmlNode* out_node) {
  SerializeXmlToPb(*resource.root, out_node);
}

}  // namespace aapt
