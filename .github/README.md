# GitHub Workflows

This directory contains GitHub Actions workflows for the pg_ai_query project.

## Workflows Overview

### CI/CD Pipeline (`ci.yml`)

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` or `develop` branches

**What it does:**
- **Multi-platform builds** (Ubuntu 20.04/22.04, macOS 12/13)
- **Multi-PostgreSQL version support** (14, 15, 16)
- **Automated testing** with sample data
- **Static analysis** with cppcheck and clang-tidy
- **Security scanning** with Trivy and TruffleHog
- **Artifact uploads** for debugging

**Jobs:**
1. `build-and-test` - Builds extension and runs basic tests
2. `static-analysis` - Runs cppcheck and clang-tidy
3. `security-scan` - Scans for vulnerabilities and secrets

### Code Formatting (`formatting.yml`)

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` or `develop` branches

**What it does:**
- **C++ formatting** with clang-format (Chromium style)
- **SQL validation** with sqlparse
- **Markdown linting** with markdownlint
- **YAML linting** with yamllint
- **Auto-formatting** on push to main (optional)

**Jobs:**
1. `format-check` - Validates C++ and SQL formatting
2. `auto-format` - Automatically formats code on main branch
3. `markdown-lint` - Lints markdown documentation
4. `yaml-lint` - Validates YAML workflow files

### Documentation (`docs.yml`)

**Triggers:**
- Push to `main` branch (docs paths)
- Pull requests to `main` branch (docs paths)
- Manual trigger (`workflow_dispatch`)

**What it does:**
- **Builds documentation** with mdBook
- **Validates HTML** output
- **Deploys to GitHub Pages** (on main branch)
- **Creates preview artifacts** (on PRs)
- **Health checks** after deployment

**Jobs:**
1. `build-docs` - Builds documentation and validates
2. `deploy-docs` - Deploys to GitHub Pages
3. `docs-preview-comment` - Comments on PRs with preview
4. `check-docs-health` - Verifies deployment
5. `docs-metrics` - Calculates documentation metrics

### Release (`release.yml`)

**Triggers:**
- Tags matching `v*` pattern
- Manual trigger with version input

**What it does:**
- **Validates release** version format
- **Builds artifacts** for multiple platforms
- **Creates GitHub release** with generated notes
- **Uploads binary packages** for each platform
- **Post-release tasks** and notifications

**Jobs:**
1. `validate-release` - Validates version and changelog
2. `build-release-artifacts` - Builds platform packages
3. `create-release` - Creates GitHub release
4. `upload-release-assets` - Uploads all artifacts
5. `post-release` - Cleanup and notifications

### Security (`security.yml`)

**Triggers:**
- Weekly schedule (Sundays 2 AM UTC)
- Push to main (dependency files)
- Pull requests (dependency files)
- Manual trigger

**What it does:**
- **Dependency security scan** with multiple tools
- **Secret detection** with TruffleHog and custom patterns
- **Vulnerability assessment** with static analysis
- **License compliance** checking
- **Security reporting** with summaries

**Jobs:**
1. `dependency-check` - Scans dependencies for vulnerabilities
2. `secret-scan` - Detects hardcoded secrets and API keys
3. `vulnerability-assessment` - Static security analysis
4. `license-compliance` - License compatibility check
5. `security-report` - Generates comprehensive report

## Configuration Files

### `.clang-format`
Defines C++ code formatting rules:
- Chromium style base
- Include organization for headers
- Consistent formatting across project

### `.gitignore`
Excludes files from version control:
- Build artifacts
- IDE files
- Configuration files with secrets
- Temporary files
- Documentation builds

## Workflow Features

### Auto-formatting
The formatting workflow can automatically format your code:
- Runs on push to `main` branch
- Uses clang-format with project settings
- Commits changes back to repository
- Respects existing code style

### Multi-platform Support
CI/CD builds and tests on:
- **Ubuntu 20.04** with PostgreSQL 14
- **Ubuntu 22.04** with PostgreSQL 15, 16
- **macOS 12** with PostgreSQL 14
- **macOS 13** with PostgreSQL 15

### Security-First Approach
- Automatic secret scanning on every commit
- Dependency vulnerability checks
- License compliance monitoring
- Security advisories integration

### Comprehensive Reporting
- Build status badges (can be added to README)
- Security assessment summaries
- Documentation metrics
- Release changelogs

## Setting Up Workflows

### Required Secrets
No secrets are required for basic workflows, but you may want to add:

- `GITHUB_TOKEN` - Automatically provided by GitHub
- Optional: Custom tokens for enhanced functionality

### GitHub Pages Setup
To enable documentation deployment:

1. Go to repository Settings → Pages
2. Set Source to "GitHub Actions"
3. Documentation will be available at: `https://username.github.io/repo-name`

### Branch Protection
Consider setting up branch protection rules:

1. Go to Settings → Branches
2. Add protection rule for `main` branch
3. Enable:
   - Require status checks to pass
   - Require pull request reviews
   - Restrict pushes to main

## Monitoring Workflows

### Status Checks
All workflows provide status checks that appear on:
- Pull requests
- Commit statuses
- Branch protection rules

### Notifications
Configure notifications in your GitHub settings to receive alerts for:
- Failed builds
- Security issues
- Release deployments

### Artifacts
Workflows generate artifacts for:
- **Build outputs** - Binary files and logs
- **Documentation previews** - For PR reviews
- **Security reports** - Compliance documentation
- **Release packages** - Distribution files

## Customization

### Modifying Workflows
To customize workflows for your needs:

1. Edit the `.github/workflows/*.yml` files
2. Adjust triggers, jobs, and steps as needed
3. Test changes on a feature branch first
4. Update this documentation

### Adding New Workflows
To add new workflows:

1. Create new `.yml` file in `.github/workflows/`
2. Follow GitHub Actions syntax
3. Add documentation here
4. Consider workflow dependencies

### Environment Variables
Common environment variables used:
- `CMAKE_BUILD_TYPE` - Build configuration (Release)
- `GITHUB_TOKEN` - Authentication token
- `GITHUB_WORKSPACE` - Workflow workspace path

## Troubleshooting

### Common Issues

**Build Failures:**
- Check PostgreSQL version compatibility
- Verify dependencies are installed
- Review CMake configuration

**Formatting Failures:**
- Run clang-format locally first
- Check for non-ASCII characters
- Verify file permissions

**Documentation Build Issues:**
- Ensure mdBook is installed
- Check for broken links
- Validate markdown syntax

**Security Scan Failures:**
- Review detected issues carefully
- Add exceptions for false positives
- Update dependencies if vulnerable

### Getting Help

- Check workflow logs in the Actions tab
- Review individual job outputs
- Compare with successful runs
- Open issues for persistent problems

## Best Practices

### For Contributors
- Run formatting locally before committing
- Test builds on multiple platforms when possible
- Keep documentation updated with code changes
- Review security scan results

### For Maintainers
- Monitor workflow performance regularly
- Update dependencies periodically
- Review and merge auto-formatting commits
- Address security issues promptly

### For Releases
- Follow semantic versioning
- Update CHANGELOG.md before releases
- Test release process on feature branches
- Verify all artifacts are generated correctly

---

These workflows ensure code quality, security, and reliable releases while maintaining comprehensive documentation and multi-platform support.