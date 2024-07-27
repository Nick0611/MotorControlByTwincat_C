#include "UIHelpDoc.h"
#ifdef UI_CFG_BUILD_DOC
#include "imgui/imgui.h"
#include "imgui_markdown/imgui_markdown.h"
#include "UIFontLoader.h"
#include "IconsFontAwesome5.h"
#include "UIImage.h"
#include "UITextureLoader.h"
#include "UIUtils.h"
#include "UILog.h"
#include "UICfgParser.h"

// Following includes for Windows LinkCallback
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shellapi.h>
#include <string>

void LinkCallback(ImGui::MarkdownLinkCallbackData data_)
{
	std::string url(data_.link, data_.linkLength);
	if (!data_.isImage)
	{
		ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
}

inline ImGui::MarkdownImageData ImageCallback(ImGui::MarkdownLinkCallbackData data_)
{
	std::string image_name(data_.link, data_.linkLength);
	ImGui::MarkdownImageData imageData; // 默认isValid为false
	// 如果文件不存在，直接返回
	if (!UIUtils::Inst()->IsFileExists(image_name))
	{
		return imageData;
	}
	// 动态加载需要显示的图片（只加载一次）
	UIHelpDoc* doc = (UIHelpDoc * )data_.userData;
	UIImageBase* image = doc->GetImage(image_name);
	if (!image)
	{
		image = UIImageFactory::LoadFromFile(image_name.c_str());
		if (!image)
		{
			return imageData;
		}
		doc->AddImage(image_name, image);
	}

	imageData.isValid = true;
	imageData.useLinkCallback = false;
	imageData.user_texture_id = (void*)(intptr_t)image->GetTextureID();
	imageData.size = ImVec2(1.0f * image->GetWidth(), 1.0f * image->GetHeight());

	// For image resize when available size.x > image width, add
	ImVec2 const contentSize = ImGui::GetContentRegionAvail();
	if (imageData.size.x > contentSize.x)
	{
		float const ratio = imageData.size.y / imageData.size.x;
		imageData.size.x = contentSize.x;
		imageData.size.y = contentSize.x * ratio;
	}

	return imageData;
}

void MarkdownFormatCallback(const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_)
{
	// Call the default first so any settings can be overwritten by our implementation.
	// Alternatively could be called or not called in a switch statement on a case by case basis.
	// See defaultMarkdownFormatCallback definition for furhter examples of how to use it.
	ImGui::defaultMarkdownFormatCallback(markdownFormatInfo_, start_);

	switch (markdownFormatInfo_.type)
	{
		// example: change the colour of heading level 2
	case ImGui::MarkdownFormatType::HEADING:
	{
		if (markdownFormatInfo_.level <= 3)
		{
			if (start_)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
			}
			else
			{
				ImGui::PopStyleColor();
			}
		}
		break;
	}
	default:
	{
		break;
	}
	}
}


//////////////////////////////////////////////////////////////////////////
UIHelpDoc::UIHelpDoc(UIGLWindow* main_win, const char* title): UIBaseWindow(main_win, title)
{
	std::string content;
	if (!g_cfg->m_help_tree.intro.file.empty())
	{
		if (UIUtils::Inst()->ReadTxtFile(g_cfg->m_help_tree.intro.file, content))
		{
			m_map_content.insert(std::make_pair(g_cfg->m_help_tree.intro.title, content));
		}
		else
		{
			UI_ERROR("failed to open file: %s", g_cfg->m_help_tree.intro.file.c_str());
		}
	}
	for (auto && item: g_cfg->m_help_tree.contents)
	{
		for (auto && child: item.child)
		{
			if (UIUtils::Inst()->ReadTxtFile(child.file, content))
			{
				m_map_content.insert(std::make_pair(child.title, content));
			}
			else
			{
				UI_ERROR("failed to open file: %s", child.file.c_str());
			}
		}
	}
}

UIHelpDoc::~UIHelpDoc()
{
	for (auto iter: m_map_images)
	{
		delete iter.second;
	}
}

void UIHelpDoc::Draw()
{
	if (!m_show)
	{
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
	if (!ImGui::Begin(m_win_title, &m_show, /*ImGuiWindowFlags_AlwaysAutoResize | */ImGuiWindowFlags_NoCollapse))
	{
		ImGui::End();
		return;
	}

	if (ImGui::BeginChild(u8"help_tree", ImVec2(300.0f, 0.0f), true))
	{
		if (!g_cfg->m_help_tree.intro.title.empty() && 
			ImGui::Selectable(g_cfg->m_help_tree.intro.title.c_str(), m_sct == g_cfg->m_help_tree.intro.title))
		{
			m_sct = g_cfg->m_help_tree.intro.title;
		}
		static ImGuiTreeNodeFlags tree_base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
			ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		for (auto&& item : g_cfg->m_help_tree.contents)
		{
			if (ImGui::TreeNodeEx(item.menu.title.c_str(), tree_base_flags))
			{
				for (auto&& child : item.child)
				{
					if (ImGui::Selectable(child.title.c_str(), m_sct == child.title))
					{
						m_sct = child.title;
					}
				}
				ImGui::TreePop();
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine(0, 2);
	if (ImGui::BeginChild(u8"help_content", ImVec2(0.0f, 0.0f), true))
	{
		ShowContent(m_sct);
	}
	ImGui::EndChild();
	ImGui::End();
}

bool UIHelpDoc::HasImage(const std::string& name)
{
	return m_map_images.find(name) != m_map_images.end();
}

void UIHelpDoc::AddImage(const std::string& name, UIImageBase* image)
{
	m_map_images.insert(std::make_pair(name, image));
}

UIImageBase* UIHelpDoc::GetImage(const std::string& name)
{
	auto iter = m_map_images.find(name);
	if (iter != m_map_images.end())
	{
		return iter->second;
	}
	return nullptr;
}

void UIHelpDoc::ShowContent(const std::string& title)
{
	auto iter = m_map_content.find(title);
	if (iter == m_map_content.end())
	{
		return;
	}
	const std::string& doc = iter->second;
	ImGui::MarkdownConfig mdConfig;

	mdConfig.linkCallback = LinkCallback;
	mdConfig.tooltipCallback = ImGui::defaultMarkdownTooltipCallback;
	mdConfig.imageCallback = ImageCallback;
	mdConfig.linkIcon = ICON_FA_LINK;
	mdConfig.headingFormats[0] = { UIFontLoader::Inst()->GetFontH1(), true };
	mdConfig.headingFormats[1] = { UIFontLoader::Inst()->GetFontH2(), true };
	mdConfig.headingFormats[2] = { UIFontLoader::Inst()->GetFontH3(), false };
	mdConfig.userData = this;
	mdConfig.formatCallback = MarkdownFormatCallback;
	ImGui::Markdown(doc.c_str(), doc.length(), mdConfig);
}
#endif